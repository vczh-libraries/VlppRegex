/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#include "RegexExpression.h"

namespace vl
{
	namespace regex_internal
	{
		class NormalizedCharSet
		{
		public:
			CharRange::List			ranges;
		};

/***********************************************************************
CharSetAlgorithm
***********************************************************************/

		class CharSetAlgorithm : public RegexExpressionAlgorithm<void, NormalizedCharSet*>
		{
		public:
			virtual void Process(CharSetExpression* expression, NormalizedCharSet* target, CharRange range) = 0;

			void Loop(CharSetExpression* expression, CharRange::List& ranges, NormalizedCharSet* target)
			{
				if (expression->reverse)
				{
					wchar_t begin = 1;
					for (vint i = 0; i < ranges.Count(); i++)
					{
						CharRange range = ranges[i];
						if (range.begin > begin)
						{
							Process(expression, target, CharRange(begin, range.begin - 1));
						}
						begin = range.end + 1;
					}
					if (begin <= 65535)
					{
						Process(expression, target, CharRange(begin, 65535));
					}
				}
				else
				{
					for (vint i = 0; i < ranges.Count(); i++)
					{
						Process(expression, target, ranges[i]);
					}
				}
			}

			void Apply(LoopExpression* expression, NormalizedCharSet* target)
			{
				Invoke(expression->expression, target);
			}

			void Apply(SequenceExpression* expression, NormalizedCharSet* target)
			{
				Invoke(expression->left, target);
				Invoke(expression->right, target);
			}

			void Apply(AlternateExpression* expression, NormalizedCharSet* target)
			{
				Invoke(expression->left, target);
				Invoke(expression->right, target);
			}

			void Apply(BeginExpression* expression, NormalizedCharSet* target)
			{
			}

			void Apply(EndExpression* expression, NormalizedCharSet* target)
			{
			}

			void Apply(CaptureExpression* expression, NormalizedCharSet* target)
			{
				Invoke(expression->expression, target);
			}

			void Apply(MatchExpression* expression, NormalizedCharSet* target)
			{
			}

			void Apply(PositiveExpression* expression, NormalizedCharSet* target)
			{
				Invoke(expression->expression, target);
			}

			void Apply(NegativeExpression* expression, NormalizedCharSet* target)
			{
				Invoke(expression->expression, target);
			}

			void Apply(UsingExpression* expression, NormalizedCharSet* target)
			{
			}
		};

/***********************************************************************
BuildNormalizedCharSetAlgorithm
***********************************************************************/

		class BuildNormalizedCharSetAlgorithm : public CharSetAlgorithm
		{
		public:
			void Process(CharSetExpression* expression, NormalizedCharSet* target, CharRange range)
			{
				vint index = 0;
				while (index < target->ranges.Count())
				{
					CharRange current = target->ranges[index];
					if (current<range || current>range)
					{
						index++;
					}
					else if (current.begin < range.begin)
					{
						// range   :    [    ?
						// current : [       ]
						target->ranges.RemoveAt(index);
						target->ranges.Add(CharRange(current.begin, range.begin - 1));
						target->ranges.Add(CharRange(range.begin, current.end));
						index++;
					}
					else if (current.begin > range.begin)
					{
						// range  :  [       ]
						// current  :   [    ?
						target->ranges.Add(CharRange(range.begin, current.begin - 1));
						range.begin = current.begin;
					}
					else if (current.end < range.end)
					{
						// range   : [       ]
						// current : [    ]
						range.begin = current.end + 1;
						index++;
					}
					else if (current.end > range.end)
					{
						// range   : [    ]
						// current : [       ]
						target->ranges.RemoveAt(index);
						target->ranges.Add(range);
						target->ranges.Add(CharRange(range.end + 1, current.end));
						return;
					}
					else
					{
						// range   : [       ]
						// current : [       ]
						return;
					}
				}
				target->ranges.Add(range);
			}

			void Apply(CharSetExpression* expression, NormalizedCharSet* target)
			{
				Loop(expression, expression->ranges, target);
			}
		};

/***********************************************************************
SetNormalizedCharSetAlgorithm
***********************************************************************/

		class SetNormalizedCharSetAlgorithm : public CharSetAlgorithm
		{
		public:
			void Process(CharSetExpression* expression, NormalizedCharSet* target, CharRange range)
			{
				for (vint j = 0; j < target->ranges.Count(); j++)
				{
					CharRange targetRange = target->ranges[j];
					if (range.begin <= targetRange.begin && targetRange.end <= range.end)
					{
						expression->ranges.Add(targetRange);
					}
				}
			}

			void Apply(CharSetExpression* expression, NormalizedCharSet* target)
			{
				CharRange::List source;
				CopyFrom(source, expression->ranges);
				expression->ranges.Clear();
				Loop(expression, source, target);
				expression->reverse = false;
			}
		};

/***********************************************************************
Expression
***********************************************************************/

		void Expression::NormalizeCharSet(CharRange::List& subsets)
		{
			NormalizedCharSet normalized;
			BuildNormalizedCharSetAlgorithm().Invoke(this, &normalized);
			SetNormalizedCharSetAlgorithm().Invoke(this, &normalized);
			CopyFrom(subsets, normalized.ranges);
		}

		void Expression::CollectCharSet(CharRange::List& subsets)
		{
			NormalizedCharSet normalized;
			CopyFrom(normalized.ranges, subsets);
			BuildNormalizedCharSetAlgorithm().Invoke(this, &normalized);
			CopyFrom(subsets, normalized.ranges);
		}

		void Expression::ApplyCharSet(CharRange::List& subsets)
		{
			NormalizedCharSet normalized;
			CopyFrom(normalized.ranges, subsets);
			SetNormalizedCharSetAlgorithm().Invoke(this, &normalized);
		}
	}
}