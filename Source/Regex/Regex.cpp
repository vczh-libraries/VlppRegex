/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#include <VlppOS.h>
#include "Regex.h"
#include "./AST/RegexExpression.h"
#include "RegexPure.h"
#include "RegexRich.h"

namespace vl
{
	namespace regex_internal
	{
		void ReadInt(stream::IStream& inputStream, vint& value);
		void ReadInts(stream::IStream& inputStream, vint count, vint* values);
		void WriteInt(stream::IStream& outputStream, vint value);
		void WriteInts(stream::IStream& outputStream, vint count, vint* values);
	}

	namespace regex
	{
		using namespace collections;
		using namespace regex_internal;

/***********************************************************************
String Conversion
***********************************************************************/

		template<typename T>
		struct U32;

		template<>
		struct U32<wchar_t>
		{
			static constexpr U32String(*ToU32)(const WString&) = &wtou32;
			static constexpr WString(*FromU32)(const U32String&) = &u32tow;
		};

		template<>
		struct U32<char8_t>
		{
			static constexpr U32String(*ToU32)(const U8String&) = &u8tou32;
			static constexpr U8String(*FromU32)(const U32String&) = &u32tou8;
		};

		template<>
		struct U32<char16_t>
		{
			static constexpr U32String(*ToU32)(const U16String&) = &u16tou32;
			static constexpr U16String(*FromU32)(const U32String&) = &u32tou16;
		};

		template<>
		struct U32<char32_t>
		{
			static U32String ToU32(const U32String& text) { return text; }
			static U32String FromU32(const U32String& text) { return text; }
		};

/***********************************************************************
RegexMatch_<T>
***********************************************************************/
		
		template<typename T>
		RegexMatch_<T>::RegexMatch_(const ObjectString<T>& _string, PureResult* _result)
			:success(true)
			, result(_string, _result->start, _result->length)
		{
		}

		template<typename T>
		RegexMatch_<T>::RegexMatch_(const ObjectString<T>& _string, RichResult* _result)
			: success(true)
			, result(_string, _result->start, _result->length)
		{
			for (vint i = 0; i < _result->captures.Count(); i++)
			{
				CaptureRecord& capture = _result->captures[i];
				if (capture.capture == -1)
				{
					captures.Add(RegexString_<T>(_string, capture.start, capture.length));
				}
				else
				{
					groups.Add(capture.capture, RegexString_<T>(_string, capture.start, capture.length));
				}
			}
		}

		template<typename T>
		RegexMatch_<T>::RegexMatch_(const RegexString_<T>& _result)
			:success(false)
			, result(_result)
		{
		}

		template<typename T>
		bool RegexMatch_<T>::Success()const
		{
			return success;
		}

		template<typename T>
		const RegexString_<T>& RegexMatch_<T>::Result()const
		{
			return result;
		}

		template<typename T>
		const typename RegexMatch_<T>::CaptureList& RegexMatch_<T>::Captures()const
		{
			return captures;
		}

		template<typename T>
		const typename RegexMatch_<T>::CaptureGroup& RegexMatch_<T>::Groups()const
		{
			return groups;
		}

/***********************************************************************
RegexBase_
***********************************************************************/

		template<typename T>
		void RegexBase_::Process(const ObjectString<T>& text, bool keepEmpty, bool keepSuccess, bool keepFail, typename RegexMatch_<T>::List& matches)const
		{
			if (rich)
			{
				const T* start = text.Buffer();
				const T* input = start;
				RichResult result;
				while (rich->Match(input, start, result))
				{
					vint offset = input - start;
					if (keepFail)
					{
						if (result.start > offset || keepEmpty)
						{
							matches.Add(new RegexMatch_<T>(RegexString_<T>(text, offset, result.start - offset)));
						}
					}
					if (keepSuccess)
					{
						matches.Add(new RegexMatch_<T>(text, &result));
					}
					input = start + result.start + result.length;
				}
				if (keepFail)
				{
					vint remain = input - start;
					vint length = text.Length() - remain;
					if (length || keepEmpty)
					{
						matches.Add(new RegexMatch_<T>(RegexString_<T>(text, remain, length)));
					}
				}
			}
			else
			{
				const T* start = text.Buffer();
				const T* input = start;
				PureResult result;
				while (pure->Match(input, start, result))
				{
					vint offset = input - start;
					if (keepFail)
					{
						if (result.start > offset || keepEmpty)
						{
							matches.Add(new RegexMatch_<T>(RegexString_<T>(text, offset, result.start - offset)));
						}
					}
					if (keepSuccess)
					{
						matches.Add(new RegexMatch_<T>(text, &result));
					}
					input = start + result.start + result.length;
				}
				if (keepFail)
				{
					vint remain = input - start;
					vint length = text.Length() - remain;
					if (length || keepEmpty)
					{
						matches.Add(new RegexMatch_<T>(RegexString_<T>(text, remain, length)));
					}
				}
			}
		}

		RegexBase_::~RegexBase_()
		{
			if (pure) delete pure;
			if (rich) delete rich;
		}

		template<typename T>
		typename RegexMatch_<T>::Ref RegexBase_::MatchHead(const ObjectString<T>& text)const
		{
			if (rich)
			{
				RichResult result;
				if (rich->MatchHead(text.Buffer(), text.Buffer(), result))
				{
					return new RegexMatch_<T>(text, &result);
				}
				else
				{
					return 0;
				}
			}
			else
			{
				PureResult result;
				if (pure->MatchHead(text.Buffer(), text.Buffer(), result))
				{
					return new RegexMatch_<T>(text, &result);
				}
				else
				{
					return 0;
				}
			}
		}

		template<typename T>
		typename RegexMatch_<T>::Ref RegexBase_::Match(const ObjectString<T>& text)const
		{
			if (rich)
			{
				RichResult result;
				if (rich->Match(text.Buffer(), text.Buffer(), result))
				{
					return new RegexMatch_<T>(text, &result);
				}
				else
				{
					return 0;
				}
			}
			else
			{
				PureResult result;
				if (pure->Match(text.Buffer(), text.Buffer(), result))
				{
					return new RegexMatch_<T>(text, &result);
				}
				else
				{
					return 0;
				}
			}
		}

		template<typename T>
		bool RegexBase_::TestHead(const ObjectString<T>& text)const
		{
			if (pure)
			{
				PureResult result;
				return pure->MatchHead(text.Buffer(), text.Buffer(), result);
			}
			else
			{
				RichResult result;
				return rich->MatchHead(text.Buffer(), text.Buffer(), result);
			}
		}

		template<typename T>
		bool RegexBase_::Test(const ObjectString<T>& text)const
		{
			if (pure)
			{
				PureResult result;
				return pure->Match(text.Buffer(), text.Buffer(), result);
			}
			else
			{
				RichResult result;
				return rich->Match(text.Buffer(), text.Buffer(), result);
			}
		}

		template<typename T>
		void RegexBase_::Search(const ObjectString<T>& text, typename RegexMatch_<T>::List& matches)const
		{
			Process(text, false, true, false, matches);
		}

		template<typename T>
		void RegexBase_::Split(const ObjectString<T>& text, bool keepEmptyMatch, typename RegexMatch_<T>::List& matches)const
		{
			Process(text, keepEmptyMatch, false, true, matches);
		}

		template<typename T>
		void RegexBase_::Cut(const ObjectString<T>& text, bool keepEmptyMatch, typename RegexMatch_<T>::List& matches)const
		{
			Process(text, keepEmptyMatch, true, true, matches);
		}

/***********************************************************************
Regex_<T>
***********************************************************************/
		
		template<typename T>
		Regex_<T>::Regex_(const ObjectString<T>& code, bool preferPure)
		{
			CharRange::List subsets;
			RegexExpression::Ref regex = ParseRegexExpression(U32<T>::ToU32(code));
			Expression::Ref expression = regex->Merge();
			expression->NormalizeCharSet(subsets);

			bool pureRequired = false;
			bool richRequired = false;
			if (preferPure)
			{
				if (expression->HasNoExtension())
				{
					pureRequired = true;
				}
				else
				{
					if (expression->CanTreatAsPure())
					{
						pureRequired = true;
						richRequired = true;
					}
					else
					{
						richRequired = true;
					}
				}
			}
			else
			{
				richRequired = true;
			}

			try
			{
				if (pureRequired)
				{
					Dictionary<State*, State*> nfaStateMap;
					Group<State*, State*> dfaStateMap;
					Automaton::Ref eNfa = expression->GenerateEpsilonNfa();
					Automaton::Ref nfa = EpsilonNfaToNfa(eNfa, PureEpsilonChecker, nfaStateMap);
					Automaton::Ref dfa = NfaToDfa(nfa, dfaStateMap);
					pure = new PureInterpretor(dfa, subsets);
				}
				if (richRequired)
				{
					Dictionary<State*, State*> nfaStateMap;
					Group<State*, State*> dfaStateMap;
					Automaton::Ref eNfa = expression->GenerateEpsilonNfa();
					Automaton::Ref nfa = EpsilonNfaToNfa(eNfa, RichEpsilonChecker, nfaStateMap);
					Automaton::Ref dfa = NfaToDfa(nfa, dfaStateMap);
					rich = new RichInterpretor(dfa);

					for (auto&& name : rich->CaptureNames())
					{
						captureNames.Add(U32<T>::FromU32(name));
					}
				}
			}
			catch (...)
			{
				if (pure)delete pure;
				if (rich)delete rich;
				throw;
			}
		}

/***********************************************************************
RegexTokens_<T>
***********************************************************************/

		template<typename T>
		class RegexTokenEnumerator : public Object, public IEnumerator<RegexToken_<T>>
		{
		protected:
			RegexToken_<T>			token;
			vint					index = -1;

			PureInterpretor*		pure;
			const Array<vint>&		stateTokens;
			const T*				start;
			vint					codeIndex;
			RegexProc_<T>			proc;

			const T*				reading;
			vint					rowStart = 0;
			vint					columnStart = 0;
			bool					cacheAvailable = false;
			RegexToken_<T>			cacheToken;

		public:
			RegexTokenEnumerator(const RegexTokenEnumerator& enumerator)
				: token(enumerator.token)
				, index(enumerator.index)
				, pure(enumerator.pure)
				, stateTokens(enumerator.stateTokens)
				, start(enumerator.start)
				, codeIndex(enumerator.codeIndex)
				, proc(enumerator.proc)
				, reading(enumerator.reading)
				, rowStart(enumerator.rowStart)
				, columnStart(enumerator.columnStart)
				, cacheAvailable(enumerator.cacheAvailable)
				, cacheToken(enumerator.cacheToken)
			{
			}

			RegexTokenEnumerator(PureInterpretor* _pure, const Array<vint>& _stateTokens, const T* _start, vint _codeIndex, RegexProc_<T> _proc)
				:index(-1)
				, pure(_pure)
				, stateTokens(_stateTokens)
				, start(_start)
				, codeIndex(_codeIndex)
				, proc(_proc)
				, reading(_start)
			{
			}

			IEnumerator<RegexToken_<T>>* Clone()const
			{
				return new RegexTokenEnumerator<T>(*this);
			}

			const RegexToken_<T>& Current()const
			{
				return token;
			}

			vint Index()const
			{
				return index;
			}

			bool Next()
			{
				if (!cacheAvailable && !*reading) return false;
				if (cacheAvailable)
				{
					token = cacheToken;
					cacheAvailable = false;
				}
				else
				{
					token.reading = reading;
					token.start = 0;
					token.length = 0;
					token.token = -2;
					token.completeToken = true;
				}

				token.rowStart = rowStart;
				token.columnStart = columnStart;
				token.rowEnd = rowStart;
				token.columnEnd = columnStart;
				token.codeIndex = codeIndex;

				PureResult result;
				while (*reading)
				{
					vint id = -1;
					bool completeToken = true;
					if (!pure->MatchHead(reading, start, result))
					{
						result.start = reading - start;

						if (id == -1 && result.terminateState != -1)
						{
							vint state = pure->GetRelatedFinalState(result.terminateState);
							if (state != -1)
							{
								id = stateTokens[state];
							}
						}

						if (id == -1)
						{
							result.length = 1;
						}
						else
						{
							completeToken = false;
						}
					}
					else
					{
						id = stateTokens.Get(result.finalState);
					}

					if (id != -1 && proc.extendProc)
					{
						RegexProcessingToken token(result.start, result.length, id, completeToken, nullptr);
						proc.extendProc(proc.argument, reading, -1, true, token);
#if _DEBUG
						CHECK_ERROR(token.interTokenState == nullptr, L"RegexTokenEnumerator::Next()#The extendProc is only allowed to create interTokenState in RegexLexerColorizer.");
#endif
						result.length = token.length;
						id = token.token;
						completeToken = token.completeToken;
					}

					if (token.token == -2)
					{
						token.start = result.start;
						token.length = result.length;
						token.token = id;
						token.completeToken = completeToken;
					}
					else if (token.token == id && id == -1)
					{
						token.length += result.length;
					}
					else
					{
						cacheAvailable = true;
						cacheToken.reading = reading;
						cacheToken.start = result.start;
						cacheToken.length = result.length;
						cacheToken.codeIndex = codeIndex;
						cacheToken.token = id;
						cacheToken.completeToken = completeToken;
					}
					reading += result.length;

					if (cacheAvailable)
					{
						break;
					}
				}

				index++;

				for (vint i = 0; i < token.length; i++)
				{
					token.rowEnd = rowStart;
					token.columnEnd = columnStart;
					if (token.reading[i] == L'\n')
					{
						rowStart++;
						columnStart = 0;
					}
					else
					{
						columnStart++;
					}
				}
				return true;
			}

			void Reset()
			{
				index = -1;
				reading = start;
				cacheAvailable = false;
			}

			void ReadToEnd(List<RegexToken_<T>>& tokens, bool(*discard)(vint))
			{
				while (Next())
				{
					if (!discard(token.token))
					{
						tokens.Add(token);
					}
				}
			}
		};

		template<typename T>
		RegexTokens_<T>::RegexTokens_(PureInterpretor* _pure, const Array<vint>& _stateTokens, const ObjectString<T>& _code, vint _codeIndex, RegexProc_<T> _proc)
			:pure(_pure)
			, stateTokens(_stateTokens)
			, code(_code)
			, codeIndex(_codeIndex)
			, proc(_proc)
		{
		}

		template<typename T>
		RegexTokens_<T>::RegexTokens_(const RegexTokens_<T>& tokens)
			:pure(tokens.pure)
			, stateTokens(tokens.stateTokens)
			, code(tokens.code)
			, codeIndex(tokens.codeIndex)
			, proc(tokens.proc)
		{
		}

		template<typename T>
		IEnumerator<RegexToken_<T>>* RegexTokens_<T>::CreateEnumerator() const
		{
			return new RegexTokenEnumerator<T>(pure, stateTokens, code.Buffer(), codeIndex, proc);
		}

		bool DefaultDiscard(vint token)
		{
			return false;
		}

		template<typename T>
		void RegexTokens_<T>::ReadToEnd(collections::List<RegexToken_<T>>& tokens, bool(*discard)(vint))const
		{
			if (discard == 0)
			{
				discard = &DefaultDiscard;
			}
			RegexTokenEnumerator<T>(pure, stateTokens, code.Buffer(), codeIndex, proc).ReadToEnd(tokens, discard);
		}

/***********************************************************************
RegexLexerWalker_<T>
***********************************************************************/

		template<typename T>
		RegexLexerWalker_<T>::RegexLexerWalker_(PureInterpretor* _pure, const Array<vint>& _stateTokens)
			:pure(_pure)
			, stateTokens(_stateTokens)
		{
		}

		template<typename T>
		RegexLexerWalker_<T>::RegexLexerWalker_(const RegexLexerWalker_<T>& tokens)
			: pure(tokens.pure)
			, stateTokens(tokens.stateTokens)
		{
		}

		template<typename T>
		vint RegexLexerWalker_<T>::GetStartState()const
		{
			return pure->GetStartState();
		}

		template<typename T>
		vint RegexLexerWalker_<T>::GetRelatedToken(vint state)const
		{
			vint finalState = state == -1 ? -1 : pure->GetRelatedFinalState(state);
			return finalState == -1 ? -1 : stateTokens.Get(finalState);
		}

		template<typename T>
		void RegexLexerWalker_<T>::Walk(T input, vint& state, vint& token, bool& finalState, bool& previousTokenStop)const
		{
			vint previousState = state;
			token = -1;
			finalState = false;
			previousTokenStop = false;
			if (state == -1)
			{
				state = pure->GetStartState();
				previousTokenStop = true;
			}

			state = pure->Transit(input, state);
			if (state == -1)
			{
				previousTokenStop = true;
				if (previousState == -1)
				{
					finalState = true;
					return;
				}
				else if (pure->IsFinalState(previousState))
				{
					state = pure->Transit(input, pure->GetStartState());
				}
			}
			if (pure->IsFinalState(state))
			{
				token = stateTokens.Get(state);
				finalState = true;
				return;
			}
			else
			{
				finalState = state == -1;
				return;
			}
		}

		template<typename T>
		vint RegexLexerWalker_<T>::Walk(T input, vint state)const
		{
			vint token = -1;
			bool finalState = false;
			bool previousTokenStop = false;
			Walk(input, state, token, finalState, previousTokenStop);
			return state;
		}

		template<typename T>
		bool RegexLexerWalker_<T>::IsClosedToken(const T* input, vint length)const
		{
			vint state = pure->GetStartState();
			for (vint i = 0; i < length; i++)
			{
				state = pure->Transit(input[i], state);
				if (state == -1) return true;
				if (pure->IsDeadState(state)) return true;
			}
			return false;
		}

		template<typename T>
		bool RegexLexerWalker_<T>::IsClosedToken(const ObjectString<T>& input)const
		{
			return IsClosedToken(input.Buffer(), input.Length());
		}

/***********************************************************************
RegexLexerColorizer_<T>
***********************************************************************/

		template<typename T>
		RegexLexerColorizer_<T>::RegexLexerColorizer_(const RegexLexerWalker_<T>& _walker, RegexProc_<T> _proc)
			:walker(_walker)
			, proc(_proc)
		{
			internalState.currentState = walker.GetStartState();
		}

		template<typename T>
		typename RegexLexerColorizer_<T>::InternalState RegexLexerColorizer_<T>::GetInternalState()
		{
			return internalState;
		}

		template<typename T>
		void RegexLexerColorizer_<T>::SetInternalState(InternalState state)
		{
			internalState = state;
		}

		template<typename T>
		void RegexLexerColorizer_<T>::Pass(T input)
		{
			WalkOneToken(&input, 1, 0, false);
		}

		template<typename T>
		vint RegexLexerColorizer_<T>::GetStartState()const
		{
			return walker.GetStartState();
		}

		template<typename T>
		void RegexLexerColorizer_<T>::CallExtendProcAndColorizeProc(const T* input, vint length, RegexProcessingToken& token, bool colorize)
		{
			vint oldTokenLength = token.length;
			proc.extendProc(proc.argument, input + token.start, length - token.start, false, token);
#if _DEBUG
			{
				bool pausedAtTheEnd = token.start + token.length == length && !token.completeToken;
				CHECK_ERROR(
					token.completeToken || pausedAtTheEnd,
					L"RegexLexerColorizer::WalkOneToken(const char32_t*, vint, vint, bool)#The extendProc is not allowed pause before the end of the input."
				);
				CHECK_ERROR(
					token.completeToken || token.token != -1,
					L"RegexLexerColorizer::WalkOneToken(const char32_t*, vint, vint, bool)#The extendProc is not allowed to pause without a valid token id."
				);
				CHECK_ERROR(
					oldTokenLength <= token.length,
					L"RegexLexerColorizer::WalkOneToken(const char32_t*, vint, vint, bool)#The extendProc is not allowed to decrease the token length."
				);
				CHECK_ERROR(
					(token.interTokenState == nullptr) == !pausedAtTheEnd,
					L"RegexLexerColorizer::Colorize(const char32_t*, vint, void*)#The extendProc should return an inter token state object if and only if a valid token does not end at the end of the input."
				);
			}
#endif
			if ((internalState.interTokenState = token.interTokenState))
			{
				internalState.interTokenId = token.token;
			}
			if (colorize)
			{
				proc.colorizeProc(proc.argument, token.start, token.length, token.token);
			}
		}

		template<typename T>
		vint RegexLexerColorizer_<T>::WalkOneToken(const T* input, vint length, vint start, bool colorize)
		{
			if (internalState.interTokenState)
			{
				RegexProcessingToken token(-1, -1, internalState.interTokenId, false, internalState.interTokenState);
				proc.extendProc(proc.argument, input, length, false, token);
#if _DEBUG
				{
					bool pausedAtTheEnd = token.length == length && !token.completeToken;
					CHECK_ERROR(
						token.completeToken || pausedAtTheEnd,
						L"RegexLexerColorizer::WalkOneToken(const char32_t*, vint, vint, bool)#The extendProc is not allowed to pause before the end of the input."
					);
					CHECK_ERROR(
						token.completeToken || token.token == internalState.interTokenId,
						L"RegexLexerColorizer::WalkOneToken(const char32_t*, vint, vint, bool)#The extendProc is not allowed to continue pausing with a different token id."
					);
					CHECK_ERROR(
						(token.interTokenState == nullptr) == !pausedAtTheEnd,
						L"RegexLexerColorizer::Colorize(const char32_t*, vint, void*)#The extendProc should return an inter token state object if and only if a valid token does not end at the end of the input."
					);
				}
#endif
				if (colorize)
				{
					proc.colorizeProc(proc.argument, 0, token.length, token.token);
				}
				if (!(internalState.interTokenState = token.interTokenState))
				{
					internalState.interTokenId = -1;
				}
				return token.length;
			}

			vint lastFinalStateLength = 0;
			vint lastFinalStateToken = -1;
			vint lastFinalStateState = -1;

			vint tokenStartState = internalState.currentState;
			for (vint i = start; i < length; i++)
			{
				vint currentToken = -1;
				bool finalState = false;
				bool previousTokenStop = false;
				walker.Walk(input[i], internalState.currentState, currentToken, finalState, previousTokenStop);

				if (previousTokenStop)
				{
					if (proc.extendProc && lastFinalStateToken != -1)
					{
						RegexProcessingToken token(start, lastFinalStateLength, lastFinalStateToken, true, nullptr);
						CallExtendProcAndColorizeProc(input, length, token, colorize);
						if (token.completeToken)
						{
							internalState.currentState = walker.GetStartState();
						}
						return start + token.length;
					}
					else if (i == start)
					{
						if (tokenStartState == GetStartState())
						{
							if (colorize)
							{
								proc.colorizeProc(proc.argument, start, 1, -1);
							}
							internalState.currentState = walker.GetStartState();
							return i + 1;
						}
					}
					else
					{
						if (colorize)
						{
							proc.colorizeProc(proc.argument, start, lastFinalStateLength, lastFinalStateToken);
						}
						internalState.currentState = lastFinalStateState;
						return start + lastFinalStateLength;
					}
				}

				if (finalState)
				{
					lastFinalStateLength = i + 1 - start;
					lastFinalStateToken = currentToken;
					lastFinalStateState = internalState.currentState;
				}
			}

			if (lastFinalStateToken != -1 && start + lastFinalStateLength == length)
			{
				if (proc.extendProc)
				{
					RegexProcessingToken token(start, lastFinalStateLength, lastFinalStateToken, true, nullptr);
					CallExtendProcAndColorizeProc(input, length, token, colorize);
				}
				else if (colorize)
				{
					proc.colorizeProc(proc.argument, start, lastFinalStateLength, lastFinalStateToken);
				}
			}
			else if (colorize)
			{
				proc.colorizeProc(proc.argument, start, length - start, walker.GetRelatedToken(internalState.currentState));
			}
			return length;
		}

		template<typename T>
		void* RegexLexerColorizer_<T>::Colorize(const T* input, vint length)
		{
			vint index = 0;
			while (index != length)
			{
				index = WalkOneToken(input, length, index, true);
			}
			return internalState.interTokenState;
		}

/***********************************************************************
RegexLexerBase_
***********************************************************************/

		RegexLexerBase_::~RegexLexerBase_()
		{
			if (pure) delete pure;
		}

		template<typename T>
		RegexTokens_<T> RegexLexerBase_::Parse(const ObjectString<T>& code, RegexProc_<T> proc, vint codeIndex)const
		{
			pure->PrepareForRelatedFinalStateTable();
			return RegexTokens_<T>(pure, stateTokens, code, codeIndex, proc);
		}

		template<typename T>
		RegexLexerWalker_<T> RegexLexerBase_::Walk()const
		{
			pure->PrepareForRelatedFinalStateTable();
			return RegexLexerWalker_<T>(pure, stateTokens);
		}

		RegexLexerWalker_<wchar_t> RegexLexerBase_::Walk()const
		{
			pure->PrepareForRelatedFinalStateTable();
			return RegexLexerWalker_<wchar_t>(pure, stateTokens);
		}

		template<typename T>
		RegexLexerColorizer_<T> RegexLexerBase_::Colorize(RegexProc_<T> proc)const
		{
			return RegexLexerColorizer_<T>(Walk<T>(), proc);
		}

/***********************************************************************
RegexLexer_<T> (Serialization)
***********************************************************************/

		template<typename T>
		RegexLexer_<T>::RegexLexer_(stream::IStream& inputStream)
		{
			pure = new PureInterpretor(inputStream);
			vint count = 0;
			ReadInt(inputStream, count);
			stateTokens.Resize(count);
			if (count > 0)
			{
				ReadInts(inputStream, count, &stateTokens[0]);
			}
		}

		template<typename T>
		void RegexLexer_<T>::Serialize(stream::IStream& outputStream)
		{
			pure->Serialize(outputStream);
			WriteInt(outputStream, stateTokens.Count());
			if (stateTokens.Count() > 0)
			{
				WriteInts(outputStream, stateTokens.Count(), &stateTokens[0]);
			}
		}

/***********************************************************************
RegexLexer_<T>
***********************************************************************/

		template<typename T>
		RegexLexer_<T>::RegexLexer_(const collections::IEnumerable<ObjectString<T>>& tokens)
		{
			// Build DFA for all tokens
			List<Expression::Ref> expressions;
			List<Automaton::Ref> dfas;
			CharRange::List subsets;
			for (auto&& code : tokens)
			{
				RegexExpression::Ref regex = ParseRegexExpression(U32<T>::ToU32(code));
				Expression::Ref expression = regex->Merge();
				expression->CollectCharSet(subsets);
				expressions.Add(expression);
			}
			for (vint i = 0; i < expressions.Count(); i++)
			{
				Dictionary<State*, State*> nfaStateMap;
				Group<State*, State*> dfaStateMap;
				Expression::Ref expression = expressions[i];
				expression->ApplyCharSet(subsets);
				Automaton::Ref eNfa = expression->GenerateEpsilonNfa();
				Automaton::Ref nfa = EpsilonNfaToNfa(eNfa, PureEpsilonChecker, nfaStateMap);
				Automaton::Ref dfa = NfaToDfa(nfa, dfaStateMap);
				dfas.Add(dfa);
			}

			// Mark all states in DFAs
			for (vint i = 0; i < dfas.Count(); i++)
			{
				Automaton::Ref dfa = dfas[i];
				for (vint j = 0; j < dfa->states.Count(); j++)
				{
					if (dfa->states[j]->finalState)
					{
						dfa->states[j]->userData = (void*)i;
					}
					else
					{
						dfa->states[j]->userData = (void*)dfas.Count();
					}
				}
			}

			// Connect all DFAs to an e-NFA
			Automaton::Ref bigEnfa = new Automaton;
			for (vint i = 0; i < dfas.Count(); i++)
			{
				CopyFrom(bigEnfa->states, dfas[i]->states, true);
				CopyFrom(bigEnfa->transitions, dfas[i]->transitions, true);
			}
			bigEnfa->startState = bigEnfa->NewState();
			for (vint i = 0; i < dfas.Count(); i++)
			{
				bigEnfa->NewEpsilon(bigEnfa->startState, dfas[i]->startState);
			}

			// Build a single DFA out of the e-NFA
			Dictionary<State*, State*> nfaStateMap;
			Group<State*, State*> dfaStateMap;
			Automaton::Ref bigNfa = EpsilonNfaToNfa(bigEnfa, PureEpsilonChecker, nfaStateMap);
			for (vint i = 0; i < nfaStateMap.Keys().Count(); i++)
			{
				void* userData = nfaStateMap.Values().Get(i)->userData;
				nfaStateMap.Keys()[i]->userData = userData;
			}
			Automaton::Ref bigDfa = NfaToDfa(bigNfa, dfaStateMap);
			for (vint i = 0; i < dfaStateMap.Keys().Count(); i++)
			{
				void* userData = dfaStateMap.GetByIndex(i).Get(0)->userData;
				for (vint j = 1; j < dfaStateMap.GetByIndex(i).Count(); j++)
				{
					void* newData = dfaStateMap.GetByIndex(i).Get(j)->userData;
					if (userData > newData)
					{
						userData = newData;
					}
				}
				dfaStateMap.Keys()[i]->userData = userData;
			}

			// Build state machine
			pure = new PureInterpretor(bigDfa, subsets);
			stateTokens.Resize(bigDfa->states.Count());
			for (vint i = 0; i < stateTokens.Count(); i++)
			{
				void* userData = bigDfa->states[i]->userData;
				stateTokens[i] = (vint)userData;
			}
		}

/***********************************************************************
Template Instantiation
***********************************************************************/

		template class RegexString_<wchar_t>;
		template class RegexString_<char8_t>;
		template class RegexString_<char16_t>;
		template class RegexString_<char32_t>;

		template class RegexMatch_<wchar_t>;
		template class RegexMatch_<char8_t>;
		template class RegexMatch_<char16_t>;
		template class RegexMatch_<char32_t>;
		
		template RegexMatch_<wchar_t>::Ref		RegexBase_::MatchHead<wchar_t>	(const ObjectString<wchar_t>& text)const;
		template RegexMatch_<wchar_t>::Ref		RegexBase_::Match<wchar_t>		(const ObjectString<wchar_t>& text)const;
		template bool							RegexBase_::TestHead<wchar_t>	(const ObjectString<wchar_t>& text)const;
		template bool							RegexBase_::Test<wchar_t>		(const ObjectString<wchar_t>& text)const;
		template void							RegexBase_::Search<wchar_t>		(const ObjectString<wchar_t>& text, RegexMatch_<wchar_t>::List& matches)const;
		template void							RegexBase_::Split<wchar_t>		(const ObjectString<wchar_t>& text, bool keepEmptyMatch, RegexMatch_<wchar_t>::List& matches)const;
		template void							RegexBase_::Cut<wchar_t>		(const ObjectString<wchar_t>& text, bool keepEmptyMatch, RegexMatch_<wchar_t>::List& matches)const;

		template RegexMatch_<char8_t>::Ref		RegexBase_::MatchHead<char8_t>	(const ObjectString<char8_t>& text)const;
		template RegexMatch_<char8_t>::Ref		RegexBase_::Match<char8_t>		(const ObjectString<char8_t>& text)const;
		template bool							RegexBase_::TestHead<char8_t>	(const ObjectString<char8_t>& text)const;
		template bool							RegexBase_::Test<char8_t>		(const ObjectString<char8_t>& text)const;
		template void							RegexBase_::Search<char8_t>		(const ObjectString<char8_t>& text, RegexMatch_<char8_t>::List& matches)const;
		template void							RegexBase_::Split<char8_t>		(const ObjectString<char8_t>& text, bool keepEmptyMatch, RegexMatch_<char8_t>::List& matches)const;
		template void							RegexBase_::Cut<char8_t>		(const ObjectString<char8_t>& text, bool keepEmptyMatch, RegexMatch_<char8_t>::List& matches)const;

		template RegexMatch_<char16_t>::Ref		RegexBase_::MatchHead<char16_t>	(const ObjectString<char16_t>& text)const;
		template RegexMatch_<char16_t>::Ref		RegexBase_::Match<char16_t>		(const ObjectString<char16_t>& text)const;
		template bool							RegexBase_::TestHead<char16_t>	(const ObjectString<char16_t>& text)const;
		template bool							RegexBase_::Test<char16_t>		(const ObjectString<char16_t>& text)const;
		template void							RegexBase_::Search<char16_t>	(const ObjectString<char16_t>& text, RegexMatch_<char16_t>::List& matches)const;
		template void							RegexBase_::Split<char16_t>		(const ObjectString<char16_t>& text, bool keepEmptyMatch, RegexMatch_<char16_t>::List& matches)const;
		template void							RegexBase_::Cut<char16_t>		(const ObjectString<char16_t>& text, bool keepEmptyMatch, RegexMatch_<char16_t>::List& matches)const;

		template RegexMatch_<char32_t>::Ref		RegexBase_::MatchHead<char32_t>	(const ObjectString<char32_t>& text)const;
		template RegexMatch_<char32_t>::Ref		RegexBase_::Match<char32_t>		(const ObjectString<char32_t>& text)const;
		template bool							RegexBase_::TestHead<char32_t>	(const ObjectString<char32_t>& text)const;
		template bool							RegexBase_::Test<char32_t>		(const ObjectString<char32_t>& text)const;
		template void							RegexBase_::Search<char32_t>	(const ObjectString<char32_t>& text, RegexMatch_<char32_t>::List& matches)const;
		template void							RegexBase_::Split<char32_t>		(const ObjectString<char32_t>& text, bool keepEmptyMatch, RegexMatch_<char32_t>::List& matches)const;
		template void							RegexBase_::Cut<char32_t>		(const ObjectString<char32_t>& text, bool keepEmptyMatch, RegexMatch_<char32_t>::List& matches)const;

		template class Regex_<wchar_t>;
		template class Regex_<char8_t>;
		template class Regex_<char16_t>;
		template class Regex_<char32_t>;

		template class RegexTokens_<wchar_t>;
		template class RegexTokens_<char8_t>;
		template class RegexTokens_<char16_t>;
		template class RegexTokens_<char32_t>;

		template class RegexLexerWalker_<wchar_t>;
		template class RegexLexerWalker_<char8_t>;
		template class RegexLexerWalker_<char16_t>;
		template class RegexLexerWalker_<char32_t>;

		template class RegexLexerColorizer_<wchar_t>;
		template class RegexLexerColorizer_<char8_t>;
		template class RegexLexerColorizer_<char16_t>;
		template class RegexLexerColorizer_<char32_t>;

		template RegexTokens_<wchar_t>				RegexLexerBase_::Parse<wchar_t>		(const ObjectString<wchar_t>& code, RegexProc_<wchar_t> _proc, vint codeIndex)const;
		template RegexLexerWalker_<wchar_t>			RegexLexerBase_::Walk<wchar_t>		()const;
		template RegexLexerColorizer_<wchar_t>		RegexLexerBase_::Colorize<wchar_t>	(RegexProc_<wchar_t> _proc)const;

		template RegexTokens_<char8_t>				RegexLexerBase_::Parse<char8_t>		(const ObjectString<char8_t>& code, RegexProc_<char8_t> _proc, vint codeIndex)const;
		template RegexLexerWalker_<char8_t>			RegexLexerBase_::Walk<char8_t>		()const;
		template RegexLexerColorizer_<char8_t>		RegexLexerBase_::Colorize<char8_t>	(RegexProc_<char8_t> _proc)const;

		template RegexTokens_<char16_t>				RegexLexerBase_::Parse<char16_t>	(const ObjectString<char16_t>& code, RegexProc_<char16_t> _proc, vint codeIndex)const;
		template RegexLexerWalker_<char16_t>		RegexLexerBase_::Walk<char16_t>		()const;
		template RegexLexerColorizer_<char16_t>		RegexLexerBase_::Colorize<char16_t>	(RegexProc_<char16_t> _proc)const;

		template RegexTokens_<char32_t>				RegexLexerBase_::Parse<char32_t>	(const ObjectString<char32_t>& code, RegexProc_<char32_t> _proc, vint codeIndex)const;
		template RegexLexerWalker_<char32_t>		RegexLexerBase_::Walk<char32_t>		()const;
		template RegexLexerColorizer_<char32_t>		RegexLexerBase_::Colorize<char32_t>	(RegexProc_<char32_t> _proc)const;

		template class RegexLexer_<wchar_t>;
		template class RegexLexer_<char8_t>;
		template class RegexLexer_<char16_t>;
		template class RegexLexer_<char32_t>;
	}
}