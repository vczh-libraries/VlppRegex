/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#include <VlppOS.h>
#include "RegexPure.h"
#include "RegexCharReader.h"

namespace vl
{
	namespace regex_internal
	{
		using namespace collections;

/***********************************************************************
Read
***********************************************************************/

		void ReadInt(stream::IStream& inputStream, vint& value)
		{
#ifdef VCZH_64
			vint32_t x = 0;
			CHECK_ERROR(
				inputStream.Read(&x, sizeof(vint32_t)) == sizeof(vint32_t),
				L"Failed to deserialize RegexLexer."
				);
			value = (vint)x;
#else
			CHECK_ERROR(
				inputStream.Read(&value, sizeof(vint32_t)) == sizeof(vint32_t),
				L"Failed to deserialize RegexLexer."
				);
#endif
		}

		void ReadInts(stream::IStream& inputStream, vint count, vint* values)
		{
#ifdef VCZH_64
			Array<vint32_t> xs(count);
			CHECK_ERROR(
				inputStream.Read(&xs[0], sizeof(vint32_t) * count) == sizeof(vint32_t) * count,
				L"Failed to deserialize RegexLexer."
				);
			for (vint i = 0; i < count; i++)
			{
				values[i] = (vint)xs[i];
			}
#else
			CHECK_ERROR(
				inputStream.Read(values, sizeof(vint32_t) * count) == sizeof(vint32_t) * count,
				L"Failed to deserialize RegexLexer."
				);
#endif
		}

		void ReadBools(stream::IStream& inputStream, vint count, bool* values)
		{
			Array<vuint8_t> bits((count + 7) / 8);
			CHECK_ERROR(
				inputStream.Read(&bits[0], sizeof(vuint8_t) * bits.Count()) == sizeof(vuint8_t) * bits.Count(),
				L"Failed to deserialize RegexLexer."
			);

			for (vint i = 0; i < count; i++)
			{
				vint x = i / 8;
				vint y = i % 8;
				values[i] = ((bits[x] >> y) & 1) == 1;
			}
		}

/***********************************************************************
Write
***********************************************************************/

		void WriteInt(stream::IStream& outputStream, vint value)
		{
#ifdef VCZH_64
			vint32_t x = (vint32_t)value;
			CHECK_ERROR(
				outputStream.Write(&x, sizeof(vint32_t)) == sizeof(vint32_t),
				L"Failed to serialize RegexLexer."
				);
#else
			CHECK_ERROR(
				outputStream.Write(&value, sizeof(vint32_t)) == sizeof(vint32_t),
				L"Failed to serialize RegexLexer."
				);
#endif
		}

		void WriteInts(stream::IStream& outputStream, vint count, vint* values)
		{
#ifdef VCZH_64
			Array<vint32_t> xs(count);
			for (vint i = 0; i < count; i++)
			{
				xs[i] = (vint32_t)values[i];
			}
			CHECK_ERROR(
				outputStream.Write(&xs[0], sizeof(vint32_t) * count) == sizeof(vint32_t) * count,
				L"Failed to serialize RegexLexer."
				);
#else
			CHECK_ERROR(
				outputStream.Write(values, sizeof(vint32_t) * count) == sizeof(vint32_t) * count,
				L"Failed to serialize RegexLexer."
				);
#endif
		}

		void WriteBools(stream::IStream& outputStream, vint count, bool* values)
		{
			Array<vuint8_t> bits((count + 7) / 8);
			memset(&bits[0], 0, sizeof(vuint8_t) * bits.Count());

			for (vint i = 0; i < count; i++)
			{
				if (values[i])
				{
					vint x = i / 8;
					vint y = i % 8;
					bits[x] |= (vuint8_t)1 << y;
				}
			}

			CHECK_ERROR(
				outputStream.Write(&bits[0], sizeof(vuint8_t) * bits.Count()) == sizeof(vuint8_t) * bits.Count(),
				L"Failed to serialize RegexLexer."
				);
		}

/***********************************************************************
PureInterpretor (Serialization)
***********************************************************************/

		PureInterpretor::PureInterpretor(stream::IStream& inputStream)
		{
			ReadInt(inputStream, stateCount);
			ReadInt(inputStream, charSetCount);
			ReadInt(inputStream, startState);
			ReadInts(inputStream, SupportedCharCount, charMap);

			transitions = new vint[stateCount * charSetCount];
			ReadInts(inputStream, stateCount * charSetCount, transitions);

			finalState = new bool[stateCount];
			ReadBools(inputStream, stateCount, finalState);
		}

		void PureInterpretor::Serialize(stream::IStream& outputStream)
		{
			WriteInt(outputStream, stateCount);
			WriteInt(outputStream, charSetCount);
			WriteInt(outputStream, startState);
			WriteInts(outputStream, SupportedCharCount, charMap);
			WriteInts(outputStream, stateCount * charSetCount, transitions);
			WriteBools(outputStream, stateCount, finalState);
		}

/***********************************************************************
PureInterpretor
***********************************************************************/

		PureInterpretor::PureInterpretor(Automaton::Ref dfa, CharRange::List& subsets)
		{
			stateCount = dfa->states.Count();
			charSetCount = subsets.Count() + 1;
			startState = dfa->states.IndexOf(dfa->startState);

			// Map char to input index (equivalent char class)
			for (vint i = 0; i < SupportedCharCount; i++)
			{
				charMap[i] = charSetCount - 1;
			}
			for (vint i = 0; i < subsets.Count(); i++)
			{
				CharRange range = subsets[i];
				for (char32_t j = range.begin; j <= range.end; j++)
				{
					if (j > MaxChar32) break;
					charMap[j] = i;
				}
			}

			// Create transitions from DFA, using input index to represent input char
			transitions = new vint[stateCount * charSetCount];
			for (vint i = 0; i < stateCount; i++)
			{
				for (vint j = 0; j < charSetCount; j++)
				{
					transitions[i * charSetCount + j] = -1;
				}

				State* state = dfa->states[i].Obj();
				for (vint j = 0; j < state->transitions.Count(); j++)
				{
					Transition* dfaTransition = state->transitions[j];
					switch (dfaTransition->type)
					{
					case Transition::Chars:
						{
							vint index = subsets.IndexOf(dfaTransition->range);
							if (index == -1)
							{
								CHECK_ERROR(false, L"PureInterpretor::PureInterpretor(Automaton::Ref, CharRange::List&)#Specified chars don't appear in the normalized char ranges.");
							}
							transitions[i * charSetCount + index] = dfa->states.IndexOf(dfaTransition->target);
						}
						break;
					default:
						CHECK_ERROR(false, L"PureInterpretor::PureInterpretor(Automaton::Ref, CharRange::List&)#PureInterpretor only accepts Transition::Chars transitions.");
					}
				}
			}

			// Mark final states
			finalState = new bool[stateCount];
			for (vint i = 0; i < stateCount; i++)
			{
				finalState[i] = dfa->states[i]->finalState;
			}
		}

		PureInterpretor::~PureInterpretor()
		{
			if (relatedFinalState) delete[] relatedFinalState;
			delete[] finalState;
			delete[] transitions;
		}

		template<typename TChar>
		bool PureInterpretor::MatchHead(const TChar* input, const TChar* start, PureResult& result)
		{
			CharReader<TChar> reader(input);
			vint currentState = startState;
			vint terminateState = -1;
			vint terminateLength = -1;

			result.start = input - start;
			result.length = -1;
			result.finalState = -1;
			result.terminateState = -1;

			while (currentState != -1)
			{
				auto c = reader.Read();

				terminateState = currentState;
				terminateLength = reader.Index();
				if (finalState[currentState])
				{
					result.length = terminateLength;
					result.finalState = currentState;
				}

				if (!c) break;
				if (c >= SupportedCharCount) break;

				vint charIndex = charMap[c];
				currentState = transitions[currentState * charSetCount + charIndex];
			}

			if (result.finalState == -1)
			{
				if (terminateLength > 0)
				{
					result.terminateState = terminateState;
				}
				result.length = terminateLength;
				return false;
			}
			else
			{
				return true;
			}
		}

		template<typename TChar>
		bool PureInterpretor::Match(const TChar* input, const TChar* start, PureResult& result)
		{
			CharReader<TChar> reader(input);
			while (reader.Read())
			{
				if (MatchHead(reader.Reading(), start, result))
				{
					return true;
				}
			}
			return false;
		}

		vint PureInterpretor::GetStartState()
		{
			return startState;
		}

		vint PureInterpretor::Transit(char32_t input, vint state)
		{
			if (0 <= state && state < stateCount && 0 <= input && input <= MaxChar32)
			{
				vint charIndex = charMap[input];
				vint nextState = transitions[state * charSetCount + charIndex];
				return nextState;
			}
			else
			{
				return -1;
			}
		}

		bool PureInterpretor::IsFinalState(vint state)
		{
			return 0 <= state && state < stateCount&& finalState[state];
		}

		bool PureInterpretor::IsDeadState(vint state)
		{
			if (state == -1) return true;
			for (vint i = 0; i < charSetCount; i++)
			{
				if (transitions[state * charSetCount + i] != -1)
				{
					return false;
				}
			}
			return true;
		}

		void PureInterpretor::PrepareForRelatedFinalStateTable()
		{
			if (!relatedFinalState)
			{
				relatedFinalState = new vint[stateCount];
				for (vint i = 0; i < stateCount; i++)
				{
					relatedFinalState[i] = finalState[i] ? i : -1;
				}
				while (true)
				{
					vint modifyCount = 0;
					for (vint i = 0; i < stateCount; i++)
					{
						if (relatedFinalState[i] == -1)
						{
							vint state = -1;
							for (vint j = 0; j < charSetCount; j++)
							{
								vint nextState = transitions[i * charSetCount + j];
								if (nextState != -1)
								{
									state = relatedFinalState[nextState];
									if (state != -1)
									{
										break;
									}
								}
							}
							if (state != -1)
							{
								relatedFinalState[i] = state;
								modifyCount++;
							}
						}
					}
					if (modifyCount == 0)
					{
						break;
					}
				}
			}
		}

		vint PureInterpretor::GetRelatedFinalState(vint state)
		{
			return relatedFinalState ? relatedFinalState[state] : -1;
		}

		template bool			PureInterpretor::MatchHead<wchar_t>(const wchar_t* input, const wchar_t* start, PureResult& result);
		template bool			PureInterpretor::MatchHead<char8_t>(const char8_t* input, const char8_t* start, PureResult& result);
		template bool			PureInterpretor::MatchHead<char16_t>(const char16_t* input, const char16_t* start, PureResult& result);
		template bool			PureInterpretor::MatchHead<char32_t>(const char32_t* input, const char32_t* start, PureResult& result);

		template bool			PureInterpretor::Match<wchar_t>(const wchar_t* input, const wchar_t* start, PureResult& result);
		template bool			PureInterpretor::Match<char8_t>(const char8_t* input, const char8_t* start, PureResult& result);
		template bool			PureInterpretor::Match<char16_t>(const char16_t* input, const char16_t* start, PureResult& result);
		template bool			PureInterpretor::Match<char32_t>(const char32_t* input, const char32_t* start, PureResult& result);
	}
}