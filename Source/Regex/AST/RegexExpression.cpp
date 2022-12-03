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
			RegexExpression*		regex = nullptr;
		};

		class MergeAlgorithm : public RegexExpressionAlgorithm<Ptr<Expression>, MergeParameter*>
		{
		public:
			Ptr<Expression> Apply(CharSetExpression* expression, MergeParameter* target) override
			{
				auto result = Ptr(new CharSetExpression);
				CopyFrom(result->ranges, expression->ranges);
				result->reverse = expression->reverse;
				return result;
			}

			Ptr<Expression> Apply(LoopExpression* expression, MergeParameter* target) override
			{
				auto result = Ptr(new LoopExpression);
				result->max = expression->max;
				result->min = expression->min;
				result->preferLong = expression->preferLong;
				result->expression = Invoke(expression->expression, target);
				return result;
			}

			Ptr<Expression> Apply(SequenceExpression* expression, MergeParameter* target) override
			{
				auto result = Ptr(new SequenceExpression);
				result->left = Invoke(expression->left, target);
				result->right = Invoke(expression->right, target);
				return result;
			}

			Ptr<Expression> Apply(AlternateExpression* expression, MergeParameter* target) override
			{
				auto result = Ptr(new AlternateExpression);
				result->left = Invoke(expression->left, target);
				result->right = Invoke(expression->right, target);
				return result;
			}

			Ptr<Expression> Apply(BeginExpression* expression, MergeParameter* target) override
			{
				return Ptr(new BeginExpression);
			}

			Ptr<Expression> Apply(EndExpression* expression, MergeParameter* target) override
			{
				return Ptr(new EndExpression);
			}

			Ptr<Expression> Apply(CaptureExpression* expression, MergeParameter* target) override
			{
				auto result = Ptr(new CaptureExpression);
				result->expression = Invoke(expression->expression, target);
				result->name = expression->name;
				return result;
			}

			Ptr<Expression> Apply(MatchExpression* expression, MergeParameter* target) override
			{
				auto result = Ptr(new MatchExpression);
				result->name = expression->name;
				result->index = expression->index;
				return result;
			}

			Ptr<Expression> Apply(PositiveExpression* expression, MergeParameter* target) override
			{
				auto result = Ptr(new PositiveExpression);
				result->expression = Invoke(expression->expression, target);
				return result;
			}

			Ptr<Expression> Apply(NegativeExpression* expression, MergeParameter* target) override
			{
				auto result = Ptr(new NegativeExpression);
				result->expression = Invoke(expression->expression, target);
				return result;
			}

			Ptr<Expression> Apply(UsingExpression* expression, MergeParameter* target) override
			{
				if (target->definitions.Keys().Contains(expression->name))
				{
					Ptr<Expression> reference = target->definitions[expression->name];
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
					target->definitions.Add(expression->name, nullptr);
					Ptr<Expression> result = Invoke(target->regex->definitions[expression->name], target);
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

		Ptr<Expression> RegexExpression::Merge()
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