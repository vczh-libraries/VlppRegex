/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#include "RegexAutomaton.h"

namespace vl
{
	namespace regex_internal
	{
		using namespace collections;

/***********************************************************************
Automaton
***********************************************************************/

		Automaton::Automaton()
		{
			startState = 0;
		}

		State* Automaton::NewState()
		{
			State* state = new State;
			state->finalState = false;
			state->userData = 0;
			states.Add(state);
			return state;
		}

		Transition* Automaton::NewTransition(State* start, State* end)
		{
			Transition* transition = new Transition;
			transition->source = start;
			transition->target = end;
			start->transitions.Add(transition);
			end->inputs.Add(transition);
			transitions.Add(transition);
			return transition;
		}

		Transition* Automaton::NewChars(State* start, State* end, CharRange range)
		{
			Transition* transition = NewTransition(start, end);
			transition->type = Transition::Chars;
			transition->range = range;
			return transition;
		}

		Transition* Automaton::NewEpsilon(State* start, State* end)
		{
			Transition* transition = NewTransition(start, end);
			transition->type = Transition::Epsilon;
			return transition;
		}

		Transition* Automaton::NewBeginString(State* start, State* end)
		{
			Transition* transition = NewTransition(start, end);
			transition->type = Transition::BeginString;
			return transition;
		}

		Transition* Automaton::NewEndString(State* start, State* end)
		{
			Transition* transition = NewTransition(start, end);
			transition->type = Transition::EndString;
			return transition;
		}

		Transition* Automaton::NewNop(State* start, State* end)
		{
			Transition* transition = NewTransition(start, end);
			transition->type = Transition::Nop;
			return transition;
		}

		Transition* Automaton::NewCapture(State* start, State* end, vint capture)
		{
			Transition* transition = NewTransition(start, end);
			transition->type = Transition::Capture;
			transition->capture = capture;
			return transition;
		}

		Transition* Automaton::NewMatch(State* start, State* end, vint capture, vint index)
		{
			Transition* transition = NewTransition(start, end);
			transition->type = Transition::Match;
			transition->capture = capture;
			transition->index = index;
			return transition;
		}

		Transition* Automaton::NewPositive(State* start, State* end)
		{
			Transition* transition = NewTransition(start, end);
			transition->type = Transition::Positive;
			return transition;
		}

		Transition* Automaton::NewNegative(State* start, State* end)
		{
			Transition* transition = NewTransition(start, end);
			transition->type = Transition::Negative;
			return transition;
		}

		Transition* Automaton::NewNegativeFail(State* start, State* end)
		{
			Transition* transition = NewTransition(start, end);
			transition->type = Transition::NegativeFail;
			return transition;
		}

		Transition* Automaton::NewEnd(State* start, State* end)
		{
			Transition* transition = NewTransition(start, end);
			transition->type = Transition::End;
			return transition;
		}

/***********************************************************************
Helpers
***********************************************************************/

		bool PureEpsilonChecker(Transition* transition)
		{
			switch (transition->type)
			{
			case Transition::Epsilon:
			case Transition::Nop:
			case Transition::Capture:
			case Transition::End:
				return true;
			default:
				return false;
			}
		}

		bool RichEpsilonChecker(Transition* transition)
		{
			switch (transition->type)
			{
			case Transition::Epsilon:
				return true;
			default:
				return false;
			}
		}

		bool AreEqual(Transition* transA, Transition* transB)
		{
			if (transA->type != transB->type)return false;
			switch (transA->type)
			{
			case Transition::Chars:
				return transA->range == transB->range;
			case Transition::Capture:
				return transA->capture == transB->capture;
			case Transition::Match:
				return transA->capture == transB->capture && transA->index == transB->index;
			default:
				return true;
			}
		}

		// Collect epsilon states and non-epsilon transitions, their order are maintained to match the e-NFA
		void CollectEpsilon(State* targetState, State* sourceState, bool(*epsilonChecker)(Transition*), List<State*>& epsilonStates, List<Transition*>& transitions)
		{
			if (!epsilonStates.Contains(sourceState))
			{
				epsilonStates.Add(sourceState);
				for (vint i = 0; i < sourceState->transitions.Count(); i++)
				{
					Transition* transition = sourceState->transitions[i];
					if (epsilonChecker(transition))
					{
						if (!epsilonStates.Contains(transition->target))
						{
							if (transition->target->finalState)
							{
								targetState->finalState = true;
							}
							CollectEpsilon(targetState, transition->target, epsilonChecker, epsilonStates, transitions);
						}
					}
					else
					{
						transitions.Add(transition);
					}
				}
			}
		}

		Automaton::Ref EpsilonNfaToNfa(Automaton::Ref source, bool(*epsilonChecker)(Transition*), Dictionary<State*, State*>& nfaStateMap)
		{
			Automaton::Ref target = new Automaton;
			Dictionary<State*, State*> stateMap;	// source->target
			List<State*> epsilonStates;				// current epsilon closure
			List<Transition*> transitions;			// current non-epsilon transitions

			stateMap.Add(source->startState, target->NewState());
			nfaStateMap.Add(stateMap[source->startState], source->startState);
			target->startState = target->states[0].Obj();
			CopyFrom(target->captureNames, source->captureNames);

			for (vint i = 0; i < target->states.Count(); i++)
			{
				// Clear cache
				State* targetState = target->states[i].Obj();
				State* sourceState = nfaStateMap[targetState];
				if (sourceState->finalState)
				{
					targetState->finalState = true;
				}
				epsilonStates.Clear();
				transitions.Clear();

				// Collect epsilon states and non-epsilon transitions
				CollectEpsilon(targetState, sourceState, epsilonChecker, epsilonStates, transitions);

				// Iterate through all non-epsilon transitions
				for (vint j = 0; j < transitions.Count(); j++)
				{
					Transition* transition = transitions[j];
					// Create and map a new target state if a new non-epsilon state is found in the e-NFA
					if (!stateMap.Keys().Contains(transition->target))
					{
						stateMap.Add(transition->target, target->NewState());
						nfaStateMap.Add(stateMap[transition->target], transition->target);
					}
					// Copy transition to connect between two non-epsilon state
					Transition* newTransition = target->NewTransition(targetState, stateMap[transition->target]);
					newTransition->capture = transition->capture;
					newTransition->index = transition->index;
					newTransition->range = transition->range;
					newTransition->type = transition->type;
				}
			}
			return target;
		}

		Automaton::Ref NfaToDfa(Automaton::Ref source, Group<State*, State*>& dfaStateMap)
		{
			Automaton::Ref target = new Automaton;
			CopyFrom(target->captureNames, source->captureNames);
			State* startState = target->NewState();
			target->startState = startState;
			dfaStateMap.Add(startState, source->startState);

			for (auto currentState_ : target->states)
			{
				Group<Transition*, Transition*>			nfaClassToTransitions;
				Dictionary<Transition*, Transition*>	nfaTransitionToClass;
				List<Transition*>						orderedTransitionClasses;

				State* currentState = currentState_.Obj();

				// Iterate through all NFA states which represent the DFA state
				for (auto nfaState : dfaStateMap[currentState])
				{
					// Iterate through all transitions from those NFA states
					for (auto nfaTransition : nfaState->transitions)
					{
						Transition* transitionClass = nullptr;

						// Check if there is any key in nfaTransitions that has the same input as the current transition
						{
							vint index = nfaTransitionToClass.Keys().IndexOf(nfaTransition);
							if (index != -1) transitionClass = nfaTransitionToClass.Values()[index];
						}

						if (transitionClass == nullptr)
						{
							for (vint l = 0; l < orderedTransitionClasses.Count(); l++)
							{
								Transition* key = orderedTransitionClasses[l];
								if (AreEqual(key, nfaTransition))
								{
									transitionClass = key;
									break;
								}
							}
						}

						// Create a new key if not
						if (transitionClass == nullptr)
						{
							transitionClass = nfaTransition;
							orderedTransitionClasses.Add(transitionClass);
						}
						// Group the transition
						nfaClassToTransitions.Add(transitionClass, nfaTransition);
						nfaTransitionToClass.Add(nfaTransition, transitionClass);
					}
				}

				// Iterate through all key transition that represent all existing transition inputs from the same state
				for (auto transitionClass : orderedTransitionClasses)
				{
					auto&& equivalentTransitions = nfaClassToTransitions[transitionClass];

					// Sort all target states and keep unique
					List<State*> transitionTargets;
					CopyFrom(
						transitionTargets,
						From(equivalentTransitions)
							.Select([](auto t) { return t->target; })
							.Distinct()
						);

					// Check if these NFA states represent a created DFA state
					State* dfaState = 0;
					for (vint k = 0; k < dfaStateMap.Count(); k++)
					{
						// Compare two NFA states set
						if (CompareEnumerable(transitionTargets, dfaStateMap.GetByIndex(k)) == 0)
						{
							dfaState = dfaStateMap.Keys()[k];
						}
					}
					// Create a new DFA state if there is not
					if (!dfaState)
					{
						dfaState = target->NewState();
						for (vint k = 0; k < transitionTargets.Count(); k++)
						{
							dfaStateMap.Add(dfaState, transitionTargets[k]);
							if (transitionTargets[k]->finalState)
							{
								dfaState->finalState = true;
							}
						}
					}
					// Create corresponding DFA transition
					Transition* newTransition = target->NewTransition(currentState, dfaState);
					newTransition->capture = transitionClass->capture;
					newTransition->index = transitionClass->index;
					newTransition->range = transitionClass->range;
					newTransition->type = transitionClass->type;
				}
			}

			return target;
		}
	}
}