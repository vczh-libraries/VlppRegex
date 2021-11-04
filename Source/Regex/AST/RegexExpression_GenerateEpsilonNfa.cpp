/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#include "RegexExpression.h"

namespace vl
{
	namespace regex_internal
	{

/***********************************************************************
EpsilonNfaAlgorithm
***********************************************************************/

		class EpsilonNfaInfo
		{
		public:
			Automaton::Ref		automaton;
		};

		class EpsilonNfa
		{
		public:
			State* start;
			State* end;

			EpsilonNfa()
			{
				start = 0;
				end = 0;
			}
		};

		class EpsilonNfaAlgorithm : public RegexExpressionAlgorithm<EpsilonNfa, Automaton*>
		{
		public:
			EpsilonNfa Connect(EpsilonNfa a, EpsilonNfa b, Automaton* target)
			{
				if (a.start)
				{
					target->NewEpsilon(a.end, b.start);
					a.end = b.end;
					return a;
				}
				else
				{
					return b;
				}
			}

			EpsilonNfa Apply(CharSetExpression* expression, Automaton* target)
			{
				EpsilonNfa nfa;
				nfa.start = target->NewState();
				nfa.end = target->NewState();
				for (vint i = 0; i < expression->ranges.Count(); i++)
				{
					target->NewChars(nfa.start, nfa.end, expression->ranges[i]);
				}
				return nfa;
			}

			EpsilonNfa Apply(LoopExpression* expression, Automaton* target)
			{
				EpsilonNfa head;
				for (vint i = 0; i < expression->min; i++)
				{
					EpsilonNfa body = Invoke(expression->expression, target);
					head = Connect(head, body, target);
				}
				if (expression->max == -1)
				{
					EpsilonNfa body = Invoke(expression->expression, target);
					if (!head.start)
					{
						head.start = head.end = target->NewState();
					}
					State* loopBegin = head.end;
					State* loopEnd = target->NewState();
					if (expression->preferLong)
					{
						target->NewEpsilon(loopBegin, body.start);
						target->NewEpsilon(body.end, loopBegin);
						target->NewNop(loopBegin, loopEnd);
					}
					else
					{
						target->NewNop(loopBegin, loopEnd);
						target->NewEpsilon(loopBegin, body.start);
						target->NewEpsilon(body.end, loopBegin);
					}
					head.end = loopEnd;
				}
				else if (expression->max > expression->min)
				{
					for (vint i = expression->min; i < expression->max; i++)
					{
						EpsilonNfa body = Invoke(expression->expression, target);
						State* start = target->NewState();
						State* end = target->NewState();
						if (expression->preferLong)
						{
							target->NewEpsilon(start, body.start);
							target->NewEpsilon(body.end, end);
							target->NewNop(start, end);
						}
						else
						{
							target->NewNop(start, end);
							target->NewEpsilon(start, body.start);
							target->NewEpsilon(body.end, end);
						}
						body.start = start;
						body.end = end;
						head = Connect(head, body, target);
					}
				}
				return head;
			}

			EpsilonNfa Apply(SequenceExpression* expression, Automaton* target)
			{
				EpsilonNfa a = Invoke(expression->left, target);
				EpsilonNfa b = Invoke(expression->right, target);
				return Connect(a, b, target);
			}

			EpsilonNfa Apply(AlternateExpression* expression, Automaton* target)
			{
				EpsilonNfa result;
				result.start = target->NewState();
				result.end = target->NewState();
				EpsilonNfa a = Invoke(expression->left, target);
				EpsilonNfa b = Invoke(expression->right, target);
				target->NewEpsilon(result.start, a.start);
				target->NewEpsilon(a.end, result.end);
				target->NewEpsilon(result.start, b.start);
				target->NewEpsilon(b.end, result.end);
				return result;
			}

			EpsilonNfa Apply(BeginExpression* expression, Automaton* target)
			{
				EpsilonNfa result;
				result.start = target->NewState();
				result.end = target->NewState();
				target->NewBeginString(result.start, result.end);
				return result;
			}

			EpsilonNfa Apply(EndExpression* expression, Automaton* target)
			{
				EpsilonNfa result;
				result.start = target->NewState();
				result.end = target->NewState();
				target->NewEndString(result.start, result.end);
				return result;
			}

			EpsilonNfa Apply(CaptureExpression* expression, Automaton* target)
			{
				EpsilonNfa result;
				result.start = target->NewState();
				result.end = target->NewState();

				vint capture = -1;
				if (expression->name != U32String::Empty)
				{
					capture = target->captureNames.IndexOf(expression->name);
					if (capture == -1)
					{
						capture = target->captureNames.Count();
						target->captureNames.Add(expression->name);
					}
				}

				EpsilonNfa body = Invoke(expression->expression, target);
				target->NewCapture(result.start, body.start, capture);
				target->NewEnd(body.end, result.end);
				return result;
			}

			EpsilonNfa Apply(MatchExpression* expression, Automaton* target)
			{
				vint capture = -1;
				if (expression->name != U32String::Empty)
				{
					capture = target->captureNames.IndexOf(expression->name);
					if (capture == -1)
					{
						capture = target->captureNames.Count();
						target->captureNames.Add(expression->name);
					}
				}
				EpsilonNfa result;
				result.start = target->NewState();
				result.end = target->NewState();
				target->NewMatch(result.start, result.end, capture, expression->index);
				return result;
			}

			EpsilonNfa Apply(PositiveExpression* expression, Automaton* target)
			{
				EpsilonNfa result;
				result.start = target->NewState();
				result.end = target->NewState();
				EpsilonNfa body = Invoke(expression->expression, target);
				target->NewPositive(result.start, body.start);
				target->NewEnd(body.end, result.end);
				return result;
			}

			EpsilonNfa Apply(NegativeExpression* expression, Automaton* target)
			{
				EpsilonNfa result;
				result.start = target->NewState();
				result.end = target->NewState();
				EpsilonNfa body = Invoke(expression->expression, target);
				target->NewNegative(result.start, body.start);
				target->NewEnd(body.end, result.end);
				target->NewNegativeFail(result.start, result.end);
				return result;
			}

			EpsilonNfa Apply(UsingExpression* expression, Automaton* target)
			{
				CHECK_FAIL(L"RegexExpression::GenerateEpsilonNfa()#UsingExpression cannot create state machine.");
			}
		};

/***********************************************************************
Expression
***********************************************************************/

		Automaton::Ref Expression::GenerateEpsilonNfa()
		{
			Automaton::Ref automaton = new Automaton;
			EpsilonNfa result = EpsilonNfaAlgorithm().Invoke(this, automaton.Obj());
			automaton->startState = result.start;
			result.end->finalState = true;
			return automaton;
		}
	}
}