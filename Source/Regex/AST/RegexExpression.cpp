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
MergeAlgorithm
***********************************************************************/

		class MergeParameter
		{
		public:
			Expression::Map			definitions;
			RegexExpression* regex;
		};

		class MergeAlgorithm : public RegexExpressionAlgorithm<Expression::Ref, MergeParameter*>
		{
		public:
			Expression::Ref Apply(CharSetExpression* expression, MergeParameter* target)
			{
				Ptr<CharSetExpression> result = new CharSetExpression;
				CopyFrom(result->ranges, expression->ranges);
				result->reverse = expression->reverse;
				return result;
			}

			Expression::Ref Apply(LoopExpression* expression, MergeParameter* target)
			{
				Ptr<LoopExpression> result = new LoopExpression;
				result->max = expression->max;
				result->min = expression->min;
				result->preferLong = expression->preferLong;
				result->expression = Invoke(expression->expression, target);
				return result;
			}

			Expression::Ref Apply(SequenceExpression* expression, MergeParameter* target)
			{
				Ptr<SequenceExpression> result = new SequenceExpression;
				result->left = Invoke(expression->left, target);
				result->right = Invoke(expression->right, target);
				return result;
			}

			Expression::Ref Apply(AlternateExpression* expression, MergeParameter* target)
			{
				Ptr<AlternateExpression> result = new AlternateExpression;
				result->left = Invoke(expression->left, target);
				result->right = Invoke(expression->right, target);
				return result;
			}

			Expression::Ref Apply(BeginExpression* expression, MergeParameter* target)
			{
				return new BeginExpression;
			}

			Expression::Ref Apply(EndExpression* expression, MergeParameter* target)
			{
				return new EndExpression;
			}

			Expression::Ref Apply(CaptureExpression* expression, MergeParameter* target)
			{
				Ptr<CaptureExpression> result = new CaptureExpression;
				result->expression = Invoke(expression->expression, target);
				result->name = expression->name;
				return result;
			}

			Expression::Ref Apply(MatchExpression* expression, MergeParameter* target)
			{
				Ptr<MatchExpression> result = new MatchExpression;
				result->name = expression->name;
				result->index = expression->index;
				return result;
			}

			Expression::Ref Apply(PositiveExpression* expression, MergeParameter* target)
			{
				Ptr<PositiveExpression> result = new PositiveExpression;
				result->expression = Invoke(expression->expression, target);
				return result;
			}

			Expression::Ref Apply(NegativeExpression* expression, MergeParameter* target)
			{
				Ptr<NegativeExpression> result = new NegativeExpression;
				result->expression = Invoke(expression->expression, target);
				return result;
			}

			Expression::Ref Apply(UsingExpression* expression, MergeParameter* target)
			{
				if (target->definitions.Keys().Contains(expression->name))
				{
					Expression::Ref reference = target->definitions[expression->name];
					if (reference)
					{
						return reference;
					}
					else
					{
						throw ArgumentException(L"Regular expression syntax error: Found reference loops in\"" + u32tow(expression->name) + L"\".", L"vl::regex_internal::RegexExpression::Merge", L"");
					}
				}
				else if (target->regex->definitions.Keys().Contains(expression->name))
				{
					target->definitions.Add(expression->name, 0);
					Expression::Ref result = Invoke(target->regex->definitions[expression->name], target);
					target->definitions.Set(expression->name, result);
					return result;
				}
				else
				{
					throw ArgumentException(L"Regular expression syntax error: Cannot find sub expression reference\"" + u32tow(expression->name) + L"\".", L"vl::regex_internal::RegexExpression::Merge", L"");
				}
			}
		};

/***********************************************************************
CharSetExpression
***********************************************************************/

		bool CharSetExpression::AddRangeWithConflict(CharRange range)
		{
			if (range.begin > range.end)
			{
				char32_t t = range.begin;
				range.begin = range.end;
				range.end = t;
			}
			for (vint i = 0; i < ranges.Count(); i++)
			{
				if (!(range<ranges[i] || range>ranges[i]))
				{
					return false;
				}
			}
			ranges.Add(range);
			return true;
		}

/***********************************************************************
RegexExpression
***********************************************************************/

		Expression::Ref RegexExpression::Merge()
		{
			MergeParameter merge;
			merge.regex = this;
			return MergeAlgorithm().Invoke(expression, &merge);
		}

/***********************************************************************
Expression::Apply
***********************************************************************/

		void CharSetExpression::Apply(IRegexExpressionAlgorithm& algorithm)
		{
			algorithm.Visit(this);
		}

		void LoopExpression::Apply(IRegexExpressionAlgorithm& algorithm)
		{
			algorithm.Visit(this);
		}

		void SequenceExpression::Apply(IRegexExpressionAlgorithm& algorithm)
		{
			algorithm.Visit(this);
		}

		void AlternateExpression::Apply(IRegexExpressionAlgorithm& algorithm)
		{
			algorithm.Visit(this);
		}

		void BeginExpression::Apply(IRegexExpressionAlgorithm& algorithm)
		{
			algorithm.Visit(this);
		}

		void EndExpression::Apply(IRegexExpressionAlgorithm& algorithm)
		{
			algorithm.Visit(this);
		}

		void CaptureExpression::Apply(IRegexExpressionAlgorithm& algorithm)
		{
			algorithm.Visit(this);
		}

		void MatchExpression::Apply(IRegexExpressionAlgorithm& algorithm)
		{
			algorithm.Visit(this);
		}

		void PositiveExpression::Apply(IRegexExpressionAlgorithm& algorithm)
		{
			algorithm.Visit(this);
		}

		void NegativeExpression::Apply(IRegexExpressionAlgorithm& algorithm)
		{
			algorithm.Visit(this);
		}

		void UsingExpression::Apply(IRegexExpressionAlgorithm& algorithm)
		{
			algorithm.Visit(this);
		}
	}
}