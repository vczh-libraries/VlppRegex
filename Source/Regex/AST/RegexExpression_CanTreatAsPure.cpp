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
CanTreatAsPureAlgorithm
***********************************************************************/

		class CanTreatAsPureAlgorithm : public RegexExpressionAlgorithm<bool, void*>
		{
		public:
			bool Apply(CharSetExpression* expression, void* target)
			{
				return true;
			}

			bool Apply(LoopExpression* expression, void* target)
			{
				return expression->preferLong && Invoke(expression->expression, 0);
			}

			bool Apply(SequenceExpression* expression, void* target)
			{
				return Invoke(expression->left, 0) && Invoke(expression->right, 0);
			}

			bool Apply(AlternateExpression* expression, void* target)
			{
				return Invoke(expression->left, 0) && Invoke(expression->right, 0);
			}

			bool Apply(BeginExpression* expression, void* target)
			{
				return false;
			}

			bool Apply(EndExpression* expression, void* target)
			{
				return false;
			}

			bool Apply(CaptureExpression* expression, void* target)
			{
				return Invoke(expression->expression, 0);
			}

			bool Apply(MatchExpression* expression, void* target)
			{
				return false;
			}

			bool Apply(PositiveExpression* expression, void* target)
			{
				return false;
			}

			bool Apply(NegativeExpression* expression, void* target)
			{
				return false;
			}

			bool Apply(UsingExpression* expression, void* target)
			{
				return false;
			}
		};

/***********************************************************************
Expression
***********************************************************************/

		bool Expression::CanTreatAsPure()
		{
			return CanTreatAsPureAlgorithm().Invoke(this, 0);
		}
	}
}