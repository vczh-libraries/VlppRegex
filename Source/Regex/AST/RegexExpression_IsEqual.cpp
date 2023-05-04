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
IsEqualAlgorithm
***********************************************************************/

		class IsEqualAlgorithm : public RegexExpressionAlgorithm<bool, Expression*>
		{
		public:
			bool Apply(CharSetExpression* expression, Expression* target) override
			{
				CharSetExpression* expected = dynamic_cast<CharSetExpression*>(target);
				if (expected)
				{
					if (expression->reverse != expected->reverse)return false;
					if (expression->ranges.Count() != expected->ranges.Count())return false;
					// TODO: (enumerable) foreach:indexed
					for (vint i = 0; i < expression->ranges.Count(); i++)
					{
						if (expression->ranges[i] != expected->ranges[i])return false;
					}
					return true;
				}
				return false;
			}

			bool Apply(LoopExpression* expression, Expression* target) override
			{
				LoopExpression* expected = dynamic_cast<LoopExpression*>(target);
				if (expected)
				{
					if (expression->min != expected->min)return false;
					if (expression->max != expected->max)return false;
					if (expression->preferLong != expected->preferLong)return false;
					if (!Invoke(expression->expression, expected->expression.Obj()))return false;
					return true;
				}
				return false;
			}

			bool Apply(SequenceExpression* expression, Expression* target) override
			{
				SequenceExpression* expected = dynamic_cast<SequenceExpression*>(target);
				if (expected)
				{
					if (!Invoke(expression->left, expected->left.Obj()))return false;
					if (!Invoke(expression->right, expected->right.Obj()))return false;
					return true;
				}
				return false;
			}

			bool Apply(AlternateExpression* expression, Expression* target) override
			{
				AlternateExpression* expected = dynamic_cast<AlternateExpression*>(target);
				if (expected)
				{
					if (!Invoke(expression->left, expected->left.Obj()))return false;
					if (!Invoke(expression->right, expected->right.Obj()))return false;
					return true;
				}
				return false;
			}

			bool Apply(BeginExpression* expression, Expression* target) override
			{
				BeginExpression* expected = dynamic_cast<BeginExpression*>(target);
				if (expected)
				{
					return true;
				}
				return false;
			}

			bool Apply(EndExpression* expression, Expression* target) override
			{
				EndExpression* expected = dynamic_cast<EndExpression*>(target);
				if (expected)
				{
					return true;
				}
				return false;
			}

			bool Apply(CaptureExpression* expression, Expression* target) override
			{
				CaptureExpression* expected = dynamic_cast<CaptureExpression*>(target);
				if (expected)
				{
					if (expression->name != expected->name)return false;
					if (!Invoke(expression->expression, expected->expression.Obj()))return false;
					return true;
				}
				return false;
			}

			bool Apply(MatchExpression* expression, Expression* target) override
			{
				MatchExpression* expected = dynamic_cast<MatchExpression*>(target);
				if (expected)
				{
					if (expression->name != expected->name)return false;
					if (expression->index != expected->index)return false;
					return true;
				}
				return false;
			}

			bool Apply(PositiveExpression* expression, Expression* target) override
			{
				PositiveExpression* expected = dynamic_cast<PositiveExpression*>(target);
				if (expected)
				{
					if (!Invoke(expression->expression, expected->expression.Obj()))return false;
					return true;
				}
				return false;
			}

			bool Apply(NegativeExpression* expression, Expression* target) override
			{
				NegativeExpression* expected = dynamic_cast<NegativeExpression*>(target);
				if (expected)
				{
					if (!Invoke(expression->expression, expected->expression.Obj()))return false;
					return true;
				}
				return false;
			}

			bool Apply(UsingExpression* expression, Expression* target) override
			{
				UsingExpression* expected = dynamic_cast<UsingExpression*>(target);
				if (expected)
				{
					if (expression->name != expected->name)return false;
					return true;
				}
				return false;
			}
		};

/***********************************************************************
Expression
***********************************************************************/

		bool Expression::IsEqual(vl::regex_internal::Expression* expression)
		{
			return IsEqualAlgorithm().Invoke(this, expression);
		}
	}
}