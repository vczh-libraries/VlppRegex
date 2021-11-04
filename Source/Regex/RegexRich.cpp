/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#include "RegexRich.h"
#include "RegexCharReader.h"

namespace vl
{
	namespace regex_internal
	{

/***********************************************************************
Data Structures for Backtracking
***********************************************************************/

		enum class StateStoreType
		{
			Positive,
			Negative,
			Other
		};

		template<typename TChar>
		class StateSaver
		{
		public:

			CharReader<TChar>		reader;										// Current reading position
			char32_t				ch;											// Current character
			State*					currentState;								// Current state
			vint					minTransition = 0;							// The first transition to backtrack
			vint					captureCount = 0;							// Available capture count			(the list size may larger than this)
			vint					stateSaverCount = 0;						// Available saver count			(the list size may larger than this)
			vint					extensionSaverAvailable = -1;				// Available extension saver count	(the list size may larger than this)
			vint					extensionSaverCount = 0;					// Available extension saver count	(during executing)
			StateStoreType			storeType = StateStoreType::Other;			// Reason to keep this record

			StateSaver(const TChar* input, State* _currentState)
				: reader(input)
				, currentState(_currentState)
			{
				ch = reader.Read();
			}

			StateSaver(const StateSaver&) = default;
			StateSaver& operator=(const StateSaver&) = default;

			bool operator==(const StateSaver& saver)const
			{
				CHECK_FAIL(L"This function is only created to satisfy List<T>.");
			}

			void RestoreReaderTo(StateSaver<TChar>& saver)
			{
				saver.reader = reader;
				saver.ch = ch;
			}
		};

		template<typename TChar>
		class ExtensionSaver
		{
		public:
			CharReader<TChar>		reader;										// The reading position
			char32_t				ch;											// Current character
			vint					previous;									// Previous extension saver index
			vint					captureListIndex;							// Where to write the captured text
			Transition*				transition;									// The extension begin transition (Capture, Positive, Negative)

			ExtensionSaver(const StateSaver<TChar>& saver)
				: reader(saver.reader)
				, ch(saver.ch)
			{
			}

			ExtensionSaver(const ExtensionSaver&) = default;
			ExtensionSaver& operator=(const ExtensionSaver&) = default;

			bool operator==(const ExtensionSaver& saver)const
			{
				CHECK_FAIL(L"This function is only created to satisfy List<T>.");
			}

			void RestoreReaderTo(StateSaver<TChar>& saver)
			{
				saver.reader = reader;
				saver.ch = ch;
			}
		};
	}

	namespace regex_internal
	{
		using namespace collections;

		template<typename TChar>
		void Push(List<ExtensionSaver<TChar>>& elements, vint& available, vint& count, const ExtensionSaver<TChar>& element)
		{
			if (elements.Count() == count)
			{
				elements.Add(element);
			}
			else
			{
				elements[count] = element;
			}
			auto& current = elements[count];
			current.previous = available;
			available = count++;
		}

		template<typename TChar>
		ExtensionSaver<TChar> Pop(List<ExtensionSaver<TChar>>& elements, vint& available, vint& count)
		{
			auto& current = elements[available];
			available = current.previous;
			return current;
		}

		template<typename T, typename K>
		void PushNonSaver(List<T, K>& elements, vint& count, const T& element)
		{
			if (elements.Count() == count)
			{
				elements.Add(element);
			}
			else
			{
				elements[count] = element;
			}
			count++;
		}

		template<typename T, typename K>
		T PopNonSaver(List<T, K>& elements, vint& count)
		{
			return elements[--count];
		}
	}

	namespace regex_internal
	{
/***********************************************************************
CaptureRecord
***********************************************************************/

		bool CaptureRecord::operator==(const CaptureRecord& record)const
		{
			return capture == record.capture && start == record.start && length == record.length;
		}

/***********************************************************************
RichInterpretor
***********************************************************************/

		RichInterpretor::RichInterpretor(Automaton::Ref _dfa)
			:dfa(_dfa)
		{
			datas = new UserData[dfa->states.Count()];

			for (vint i = 0; i < dfa->states.Count(); i++)
			{
				State* state = dfa->states[i].Obj();
				vint charEdges = 0;
				vint nonCharEdges = 0;
				bool mustSave = false;
				for (vint j = 0; j < state->transitions.Count(); j++)
				{
					if (state->transitions[j]->type == Transition::Chars)
					{
						charEdges++;
					}
					else
					{
						if (state->transitions[j]->type == Transition::Negative ||
							state->transitions[j]->type == Transition::Positive)
						{
							mustSave = true;
						}
						nonCharEdges++;
					}
				}
				datas[i].NeedKeepState = mustSave || nonCharEdges > 1 || (nonCharEdges != 0 && charEdges != 0);
				state->userData = &datas[i];
			}
		}

		RichInterpretor::~RichInterpretor()
		{
			delete[] datas;
		}

		template<typename TChar>
		bool RichInterpretor::MatchHead(const TChar* input, const TChar* start, RichResult& result)
		{
			List<StateSaver<TChar>> stateSavers;
			List<ExtensionSaver<TChar>> extensionSavers;

			StateSaver<TChar> currentState(input, dfa->startState);

			while (!currentState.currentState->finalState)
			{
				bool found = false; // true means at least one transition matches the input
				StateSaver<TChar> oldState = currentState;
				// Iterate through all transitions from the current state
				for (vint i = currentState.minTransition; i < currentState.currentState->transitions.Count(); i++)
				{
					Transition* transition = currentState.currentState->transitions[i];
					switch (transition->type)
					{
					case Transition::Chars:
						{
							// match the input if the current character fall into the range
							CharRange range = transition->range;
							found =
								range.begin <= currentState.ch &&
								range.end >= currentState.ch;
							if (found)
							{
								currentState.ch = currentState.reader.Read();
							}
						}
						break;
					case Transition::BeginString:
						{
							// match the input if this is the first character, and it is not consumed
							found = currentState.reader.Index() == 0;
						}
						break;
					case Transition::EndString:
						{
							// match the input if this is after the last character, and it is not consumed
							found = currentState.ch == 0;
						}
						break;
					case Transition::Nop:
						{
							// match without any condition
							found = true;
						}
						break;
					case Transition::Capture:
						{
							// Push the capture information
							ExtensionSaver<TChar> saver(currentState);
							saver.captureListIndex = currentState.captureCount;
							saver.transition = transition;
							Push(extensionSavers, currentState.extensionSaverAvailable, currentState.extensionSaverCount, saver);

							// Push the capture record, and it will be written if the input matches the regex
							CaptureRecord capture;
							capture.capture = transition->capture;
							capture.start = currentState.reader.Index();
							capture.length = -1;
							PushNonSaver(result.captures, currentState.captureCount, capture);

							found = true;
						}
						break;
					case Transition::Match:
						{
							vint index = 0;
							for (vint j = 0; j < currentState.captureCount; j++)
							{
								CaptureRecord& capture = result.captures[j];
								// If the capture name matched
								if (capture.capture == transition->capture)
								{
									// If the capture index matched, or it is -1
									if (capture.length != -1 && (transition->index == -1 || transition->index == index))
									{
										// If the captured text matched
										if (memcmp(start + capture.start, input + currentState.reader.Index(), sizeof(TChar) * capture.length) == 0)
										{
											// Consume so much input
											vint targetIndex = currentState.reader.Index() + capture.length;
											while (currentState.reader.Index() < targetIndex)
											{
												currentState.ch = currentState.reader.Read();
											}
											CHECK_ERROR(currentState.reader.Index() == targetIndex, L"vl::regex_internal::RichInterpretor::MatchHead<TChar>(const TChar*, const TChar*, RichResult&)#Input code could be an incorrect unicode sequence.");
											found = true;
											break;
										}
									}

									// Fail if f the captured text with the specified name and index doesn't match
									if (transition->index != -1 && index == transition->index)
									{
										break;
									}
									else
									{
										index++;
									}
								}
							}
						}
						break;
					case Transition::Positive:
						{
							// Push the positive lookahead information
							ExtensionSaver<TChar> saver(currentState);
							saver.captureListIndex = -1;
							saver.transition = transition;
							Push(extensionSavers, currentState.extensionSaverAvailable, currentState.extensionSaverCount, saver);

							// Set found = true so that PushNonSaver(oldState) happens later
							oldState.storeType = StateStoreType::Positive;
							found = true;
						}
						break;
					case Transition::Negative:
						{
							// Push the positive lookahead information

							ExtensionSaver<TChar> saver(currentState);
							saver.captureListIndex = -1;
							saver.transition = transition;
							Push(extensionSavers, currentState.extensionSaverAvailable, currentState.extensionSaverCount, saver);

							// Set found = true so that PushNonSaver(oldState) happens later
							oldState.storeType = StateStoreType::Negative;
							found = true;
						}
						break;
					case Transition::NegativeFail:
						{
							// NegativeFail will be used when the nagative lookahead failed
						}
						break;
					case Transition::End:
						{
							// Find the corresponding extension saver so that we can know how to deal with a matched sub regex that ends here
							ExtensionSaver extensionSaver = Pop(extensionSavers, currentState.extensionSaverAvailable, currentState.extensionSaverCount);
							switch (extensionSaver.transition->type)
							{
							case Transition::Capture:
								{
									// Write the captured text
									CaptureRecord& capture = result.captures[extensionSaver.captureListIndex];
									capture.length = currentState.reader.Index();
									found = true;
								}
								break;
							case Transition::Positive:
								// Find the last positive lookahead state saver
								for (vint j = currentState.stateSaverCount - 1; j >= 0; j--)
								{
									auto& stateSaver = stateSavers[j];
									if (stateSaver.storeType == StateStoreType::Positive)
									{
										// restore the parsing state just before matching the positive lookahead, since positive lookahead doesn't consume input
										stateSaver.RestoreReaderTo(oldState);
										oldState.stateSaverCount = j;
										stateSaver.RestoreReaderTo(currentState);
										currentState.stateSaverCount = j;
										break;
									}
								}
								found = true;
								break;
							case Transition::Negative:
								// Find the last negative lookahead state saver
								for (vint j = currentState.stateSaverCount - 1; j >= 0; j--)
								{
									auto& stateSaver = stateSavers[j];
									if (stateSaver.storeType == StateStoreType::Negative)
									{
										// restore the parsing state just before matching the negative lookahead, since positive lookahead doesn't consume input
										oldState = stateSaver;
										oldState.storeType = StateStoreType::Other;
										currentState = stateSaver;
										currentState.storeType = StateStoreType::Other;
										i = currentState.minTransition - 1;
										break;
									}
								}
								break;
							default:;
							}
						}
						break;
					default:;
					}

					// Save the parsing state when necessary
					if (found)
					{
						UserData* data = (UserData*)currentState.currentState->userData;
						if (data->NeedKeepState)
						{
							oldState.minTransition = i + 1;
							PushNonSaver(stateSavers, currentState.stateSaverCount, oldState);
						}
						currentState.currentState = transition->target;
						currentState.minTransition = 0;
						break;
					}
				}

				// If no transition from the current state can be used
				if (!found)
				{
					// If there is a chance to do backtracking
					if (currentState.stateSaverCount)
					{
						currentState = PopNonSaver(stateSavers, currentState.stateSaverCount);
						// minTransition - 1 is always valid since the value is stored with adding 1
						// So minTransition - 1 record the transition, which is the reason the parsing state is saved
						if (currentState.currentState->transitions[currentState.minTransition - 1]->type == Transition::Negative)
						{
							// Find the next NegativeFail transition
							// Because when a negative lookahead regex failed to match, it is actually succeeded
							// Since a negative lookahead means we don't want to match this regex
							for (vint i = 0; i < currentState.currentState->transitions.Count(); i++)
							{
								Transition* transition = currentState.currentState->transitions[i];
								if (transition->type == Transition::NegativeFail)
								{
									// Restore the state to the target of NegativeFail to let the parsing continue
									currentState.currentState = transition->target;
									currentState.minTransition = 0;
									currentState.storeType = StateStoreType::Other;
									break;
								}
							}
						}
					}
					else
					{
						break;
					}
				}
			}

			if (currentState.currentState->finalState)
			{
				// Keep available captures if succeeded
				result.start = input - start;
				result.length = currentState.reader.Index();
				for (vint i = result.captures.Count() - 1; i >= currentState.captureCount; i--)
				{
					result.captures.RemoveAt(i);
				}
				return true;
			}
			else
			{
				// Clear captures if failed
				result.captures.Clear();
				return false;
			}
		}

		template<typename TChar>
		bool RichInterpretor::Match(const TChar* input, const TChar* start, RichResult& result)
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

		const List<U32String>& RichInterpretor::CaptureNames()
		{
			return dfa->captureNames;
		}

		template bool			RichInterpretor::MatchHead<wchar_t>(const wchar_t* input, const wchar_t* start, RichResult& result);
		template bool			RichInterpretor::MatchHead<char8_t>(const char8_t* input, const char8_t* start, RichResult& result);
		template bool			RichInterpretor::MatchHead<char16_t>(const char16_t* input, const char16_t* start, RichResult& result);
		template bool			RichInterpretor::MatchHead<char32_t>(const char32_t* input, const char32_t* start, RichResult& result);
								
		template bool			RichInterpretor::Match<wchar_t>(const wchar_t* input, const wchar_t* start, RichResult& result);
		template bool			RichInterpretor::Match<char8_t>(const char8_t* input, const char8_t* start, RichResult& result);
		template bool			RichInterpretor::Match<char16_t>(const char16_t* input, const char16_t* start, RichResult& result);
		template bool			RichInterpretor::Match<char32_t>(const char32_t* input, const char32_t* start, RichResult& result);
	}
}