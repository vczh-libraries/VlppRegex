/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#include "RegexPure.h"
#include "RegexCharReader.h"

namespace vl
{
	namespace regex_internal
	{

/***********************************************************************
PureInterpretor
***********************************************************************/

		PureInterpretor::PureInterpretor(Automaton::Ref dfa, CharRange::List& subsets)
			:transition(0)
			, finalState(0)
			, relatedFinalState(0)
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
			transition = new vint * [stateCount];
			for (vint i = 0; i < stateCount; i++)
			{
				transition[i] = new vint[charSetCount];
				for (vint j = 0; j < charSetCount; j++)
				{
					transition[i][j] = -1;
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
							transition[i][index] = dfa->states.IndexOf(dfaTransition->target);
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
			for (vint i = 0; i < stateCount; i++)
			{
				delete[] transition[i];
			}
			delete[] transition;
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
				currentState = transition[currentState][charIndex];
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
				vint nextState = transition[state][charIndex];
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
				if (transition[state][i] != -1)
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
								vint nextState = transition[i][j];
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