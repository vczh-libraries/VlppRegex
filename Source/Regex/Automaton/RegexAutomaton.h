/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_REGEX_REGEXAUTOMATON
#define VCZH_REGEX_REGEXAUTOMATON

#include "RegexData.h"

namespace vl
{
	namespace regex_internal
	{
		constexpr char32_t						MaxChar32 = 0x10FFFF;

		class State;
		class Transition;

		class Transition
		{
		public:
			enum Type
			{
				Chars,				// Character range transition
				Epsilon,
				BeginString,
				EndString,
				Nop,				// Non-epsilon transition with no input
				Capture,			// Begin capture transition
				Match,				// Capture matching transition
				Positive,			// Begin positive lookahead
				Negative,			// Begin negative lookahead
				NegativeFail,		// Negative lookahead failure
				End					// For Capture, Position, Negative
			};

			State*								source;
			State*								target;
			CharRange							range;
			Type								type;
			vint								capture;
			vint								index;
		};

		class State
		{
		public:
			collections::List<Transition*>		transitions;
			collections::List<Transition*>		inputs;
			bool								finalState;
			void*								userData;
		};

		class Automaton
		{
		public:
			collections::List<Ptr<State>>		states;
			collections::List<Ptr<Transition>>	transitions;
			collections::List<U32String>		captureNames;
			State*								startState;

			Automaton();

			State*								NewState();
			Transition*							NewTransition(State* start, State* end);
			Transition*							NewChars(State* start, State* end, CharRange range);
			Transition*							NewEpsilon(State* start, State* end);
			Transition*							NewBeginString(State* start, State* end);
			Transition*							NewEndString(State* start, State* end);
			Transition*							NewNop(State* start, State* end);
			Transition*							NewCapture(State* start, State* end, vint capture);
			Transition*							NewMatch(State* start, State* end, vint capture, vint index=-1);
			Transition*							NewPositive(State* start, State* end);
			Transition*							NewNegative(State* start, State* end);
			Transition*							NewNegativeFail(State* start, State* end);
			Transition*							NewEnd(State* start, State* end);
		};

		extern bool								PureEpsilonChecker(Transition* transition);
		extern bool								RichEpsilonChecker(Transition* transition);
		extern bool								AreEqual(Transition* transA, Transition* transB);
		extern Ptr<Automaton>					EpsilonNfaToNfa(Ptr<Automaton> source, bool(*epsilonChecker)(Transition*), collections::Dictionary<State*, State*>& nfaStateMap);
		extern Ptr<Automaton>					NfaToDfa(Ptr<Automaton> source, collections::Group<State*, State*>& dfaStateMap);
	}
}

#endif