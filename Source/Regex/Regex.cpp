/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#include "Regex.h"
#include "./AST/RegexExpression.h"
#include "RegexPure.h"
#include "RegexRich.h"

namespace vl
{
	namespace regex
	{
		using namespace collections;
		using namespace regex_internal;

/***********************************************************************
RegexMatch
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
					captures.Add(RegexString(_string, capture.start, capture.length));
				}
				else
				{
					groups.Add(capture.capture, RegexString(_string, capture.start, capture.length));
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
		const RegexMatch_<T>::CaptureList& RegexMatch_<T>::Captures()const
		{
			return captures;
		}

		template<typename T>
		const RegexMatch_<T>::CaptureGroup& RegexMatch_<T>::Groups()const
		{
			return groups;
		}

/***********************************************************************
Regex
***********************************************************************/

		void Regex::Process(const U32String& text, bool keepEmpty, bool keepSuccess, bool keepFail, RegexMatch::List& matches)const
		{
			if (rich)
			{
				const char32_t* start = text.Buffer();
				const char32_t* input = start;
				RichResult result;
				while (rich->Match(input, start, result))
				{
					vint offset = input - start;
					if (keepFail)
					{
						if (result.start > offset || keepEmpty)
						{
							matches.Add(new RegexMatch(RegexString(text, offset, result.start - offset)));
						}
					}
					if (keepSuccess)
					{
						matches.Add(new RegexMatch(text, &result, rich));
					}
					input = start + result.start + result.length;
				}
				if (keepFail)
				{
					vint remain = input - start;
					vint length = text.Length() - remain;
					if (length || keepEmpty)
					{
						matches.Add(new RegexMatch(RegexString(text, remain, length)));
					}
				}
			}
			else
			{
				const char32_t* start = text.Buffer();
				const char32_t* input = start;
				PureResult result;
				while (pure->Match(input, start, result))
				{
					vint offset = input - start;
					if (keepFail)
					{
						if (result.start > offset || keepEmpty)
						{
							matches.Add(new RegexMatch(RegexString(text, offset, result.start - offset)));
						}
					}
					if (keepSuccess)
					{
						matches.Add(new RegexMatch(text, &result));
					}
					input = start + result.start + result.length;
				}
				if (keepFail)
				{
					vint remain = input - start;
					vint length = text.Length() - remain;
					if (length || keepEmpty)
					{
						matches.Add(new RegexMatch(RegexString(text, remain, length)));
					}
				}
			}
		}
		
		Regex::Regex(const U32String& code, bool preferPure)
		{
			CharRange::List subsets;
			RegexExpression::Ref regex = ParseRegexExpression(code);
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
				}
			}
			catch (...)
			{
				if (pure)delete pure;
				if (rich)delete rich;
				throw;
			}
		}

		Regex::~Regex()
		{
			if (pure) delete pure;
			if (rich) delete rich;
		}

		bool Regex::IsPureMatch()const
		{
			return rich ? false : true;
		}

		bool Regex::IsPureTest()const
		{
			return pure ? true : false;
		}

		RegexMatch::Ref Regex::MatchHead(const U32String& text)const
		{
			if (rich)
			{
				RichResult result;
				if (rich->MatchHead(text.Buffer(), text.Buffer(), result))
				{
					return new RegexMatch(text, &result, rich);
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
					return new RegexMatch(text, &result);
				}
				else
				{
					return 0;
				}
			}
		}

		RegexMatch::Ref Regex::Match(const U32String& text)const
		{
			if (rich)
			{
				RichResult result;
				if (rich->Match(text.Buffer(), text.Buffer(), result))
				{
					return new RegexMatch(text, &result, rich);
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
					return new RegexMatch(text, &result);
				}
				else
				{
					return 0;
				}
			}
		}

		bool Regex::TestHead(const U32String& text)const
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

		bool Regex::Test(const U32String& text)const
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

		void Regex::Search(const U32String& text, RegexMatch::List& matches)const
		{
			Process(text, false, true, false, matches);
		}

		void Regex::Split(const U32String& text, bool keepEmptyMatch, RegexMatch::List& matches)const
		{
			Process(text, keepEmptyMatch, false, true, matches);
		}

		void Regex::Cut(const U32String& text, bool keepEmptyMatch, RegexMatch::List& matches)const
		{
			Process(text, keepEmptyMatch, true, true, matches);
		}

/***********************************************************************
RegexTokens
***********************************************************************/

		bool RegexToken::operator==(const RegexToken& _token)const
		{
			return length == _token.length && token == _token.token && reading == _token.reading;
		}

		class RegexTokenEnumerator : public Object, public IEnumerator<RegexToken>
		{
		protected:
			RegexToken				token;
			vint					index = -1;

			PureInterpretor*		pure;
			const Array<vint>&		stateTokens;
			const char32_t*			start;
			vint					codeIndex;
			RegexProc				proc;

			const char32_t*			reading;
			vint					rowStart = 0;
			vint					columnStart = 0;
			bool					cacheAvailable = false;
			RegexToken				cacheToken;

		public:
			RegexTokenEnumerator(const RegexTokenEnumerator& enumerator)
				:token(enumerator.token)
				, index(enumerator.index)
				, pure(enumerator.pure)
				, stateTokens(enumerator.stateTokens)
				, proc(enumerator.proc)
				, reading(enumerator.reading)
				, start(enumerator.start)
				, rowStart(enumerator.rowStart)
				, columnStart(enumerator.columnStart)
				, codeIndex(enumerator.codeIndex)
				, cacheAvailable(enumerator.cacheAvailable)
				, cacheToken(enumerator.cacheToken)
			{
			}

			RegexTokenEnumerator(PureInterpretor* _pure, const Array<vint>& _stateTokens, const char32_t* _start, vint _codeIndex, RegexProc _proc)
				:index(-1)
				, pure(_pure)
				, stateTokens(_stateTokens)
				, start(_start)
				, codeIndex(_codeIndex)
				, proc(_proc)
				, reading(_start)
			{
			}

			IEnumerator<RegexToken>* Clone()const
			{
				return new RegexTokenEnumerator(*this);
			}

			const RegexToken& Current()const
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

			void ReadToEnd(List<RegexToken>& tokens, bool(*discard)(vint))
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

		RegexTokens::RegexTokens(PureInterpretor* _pure, const Array<vint>& _stateTokens, const U32String& _code, vint _codeIndex, RegexProc _proc)
			:pure(_pure)
			, stateTokens(_stateTokens)
			, code(_code)
			, codeIndex(_codeIndex)
			, proc(_proc)
		{
		}

		RegexTokens::RegexTokens(const RegexTokens& tokens)
			:pure(tokens.pure)
			, stateTokens(tokens.stateTokens)
			, code(tokens.code)
			, codeIndex(tokens.codeIndex)
			, proc(tokens.proc)
		{
		}

		IEnumerator<RegexToken>* RegexTokens::CreateEnumerator() const
		{
			return new RegexTokenEnumerator(pure, stateTokens, code.Buffer(), codeIndex, proc);
		}

		bool DefaultDiscard(vint token)
		{
			return false;
		}

		void RegexTokens::ReadToEnd(collections::List<RegexToken>& tokens, bool(*discard)(vint))const
		{
			if(discard==0)
			{
				discard=&DefaultDiscard;
			}
			RegexTokenEnumerator(pure, stateTokens, code.Buffer(), codeIndex, proc).ReadToEnd(tokens, discard);
		}

/***********************************************************************
RegexLexerWalker
***********************************************************************/

		RegexLexerWalker::RegexLexerWalker(PureInterpretor* _pure, const Array<vint>& _stateTokens)
			:pure(_pure)
			, stateTokens(_stateTokens)
		{
		}

		RegexLexerWalker::RegexLexerWalker(const RegexLexerWalker& tokens)
			: pure(tokens.pure)
			, stateTokens(tokens.stateTokens)
		{
		}

		RegexLexerWalker::~RegexLexerWalker()
		{
		}

		RegexTokens::~RegexTokens()
		{
		}

		vint RegexLexerWalker::GetStartState()const
		{
			return pure->GetStartState();
		}

		vint RegexLexerWalker::GetRelatedToken(vint state)const
		{
			vint finalState = state == -1 ? -1 : pure->GetRelatedFinalState(state);
			return finalState == -1 ? -1 : stateTokens.Get(finalState);
		}

		void RegexLexerWalker::Walk(char32_t input, vint& state, vint& token, bool& finalState, bool& previousTokenStop)const
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

		vint RegexLexerWalker::Walk(char32_t input, vint state)const
		{
			vint token = -1;
			bool finalState = false;
			bool previousTokenStop = false;
			Walk(input, state, token, finalState, previousTokenStop);
			return state;
		}

		bool RegexLexerWalker::IsClosedToken(const char32_t* input, vint length)const
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

		bool RegexLexerWalker::IsClosedToken(const U32String& input)const
		{
			return IsClosedToken(input.Buffer(), input.Length());
		}

/***********************************************************************
RegexLexerColorizer
***********************************************************************/

		RegexLexerColorizer::RegexLexerColorizer(const RegexLexerWalker& _walker, RegexProc _proc)
			:walker(_walker)
			, proc(_proc)
		{
			internalState.currentState = walker.GetStartState();
		}

		RegexLexerColorizer::RegexLexerColorizer(const RegexLexerColorizer& colorizer)
			:walker(colorizer.walker)
			, proc(colorizer.proc)
			, internalState(colorizer.internalState)
		{
		}

		RegexLexerColorizer::~RegexLexerColorizer()
		{
		}

		RegexLexerColorizer::InternalState RegexLexerColorizer::GetInternalState()
		{
			return internalState;
		}
		void RegexLexerColorizer::SetInternalState(InternalState state)
		{
			internalState = state;
		}

		void RegexLexerColorizer::Pass(char32_t input)
		{
			WalkOneToken(&input, 1, 0, false);
		}

		vint RegexLexerColorizer::GetStartState()const
		{
			return walker.GetStartState();
		}

		void RegexLexerColorizer::CallExtendProcAndColorizeProc(const char32_t* input, vint length, RegexProcessingToken& token, bool colorize)
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

		vint RegexLexerColorizer::WalkOneToken(const char32_t* input, vint length, vint start, bool colorize)
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

		void* RegexLexerColorizer::Colorize(const char32_t* input, vint length)
		{
			vint index = 0;
			while (index != length)
			{
				index = WalkOneToken(input, length, index, true);
			}
			return internalState.interTokenState;
		}

/***********************************************************************
RegexLexer
***********************************************************************/

		RegexLexer::RegexLexer(const collections::IEnumerable<U32String>& tokens, RegexProc _proc)
			:proc(_proc)
		{
			// Build DFA for all tokens
			List<Expression::Ref> expressions;
			List<Automaton::Ref> dfas;
			CharRange::List subsets;
			for (auto&& code : tokens)
			{
				RegexExpression::Ref regex = ParseRegexExpression(code);
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
				CopyFrom(bigEnfa->states, dfas[i]->states);
				CopyFrom(bigEnfa->transitions, dfas[i]->transitions);
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

		RegexLexer::~RegexLexer()
		{
			if (pure) delete pure;
		}

		RegexTokens RegexLexer::Parse(const U32String& code, vint codeIndex)const
		{
			pure->PrepareForRelatedFinalStateTable();
			return RegexTokens(pure, stateTokens, code, codeIndex, proc);
		}

		RegexLexerWalker RegexLexer::Walk()const
		{
			pure->PrepareForRelatedFinalStateTable();
			return RegexLexerWalker(pure, stateTokens);
		}

		RegexLexerColorizer RegexLexer::Colorize()const
		{
			return RegexLexerColorizer(Walk(), proc);
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
	}
}