/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#include "RegexWriter.h"

namespace vl
{
	namespace regex
	{
		using namespace vl::regex_internal;

/***********************************************************************
RegexNode
***********************************************************************/

		RegexNode::RegexNode(Ptr<vl::regex_internal::Expression> _expression)
			:expression(_expression)
		{
		}

		RegexNode RegexNode::Some()const
		{
			return Loop(1, -1);
		}

		RegexNode RegexNode::Any()const
		{
			return Loop(0, -1);
		}

		RegexNode RegexNode::Opt()const
		{
			return Loop(0, 1);
		}

		RegexNode RegexNode::Loop(vint min, vint max)const
		{
			auto target = Ptr(new LoopExpression);
			target->min = min;
			target->max = max;
			target->preferLong = true;
			target->expression = expression;
			return RegexNode(target);
		}

		RegexNode RegexNode::AtLeast(vint min)const
		{
			return Loop(min, -1);
		}

		RegexNode RegexNode::operator+(const RegexNode& node)const
		{
			auto target = Ptr(new SequenceExpression);
			target->left = expression;
			target->right = node.expression;
			return RegexNode(target);
		}

		RegexNode RegexNode::operator|(const RegexNode& node)const
		{
			auto target = Ptr(new AlternateExpression);
			target->left = expression;
			target->right = node.expression;
			return RegexNode(target);
		}

		RegexNode RegexNode::operator+()const
		{
			auto target = Ptr(new PositiveExpression);
			target->expression = expression;
			return RegexNode(target);
		}

		RegexNode RegexNode::operator-()const
		{
			auto target = Ptr(new NegativeExpression);
			target->expression = expression;
			return RegexNode(target);
		}

		RegexNode RegexNode::operator!()const
		{
			auto source = dynamic_cast<CharSetExpression*>(expression.Obj());
			CHECK_ERROR(source, L"RegexNode::operator!()#operator ! can only applies on charset expressions.");
			auto target = Ptr(new CharSetExpression);
			CopyFrom(target->ranges, source->ranges);
			target->reverse = !source->reverse;
			return RegexNode(target);
		}

		RegexNode RegexNode::operator%(const RegexNode& node)const
		{
			auto left = dynamic_cast<CharSetExpression*>(expression.Obj());
			auto right = dynamic_cast<CharSetExpression*>(node.expression.Obj());
			CHECK_ERROR(left && right && !left->reverse && !right->reverse, L"RegexNode::operator%(const RegexNode&)#operator % only connects non-reverse charset expressions.");
			auto target = Ptr(new CharSetExpression);
			target->reverse = false;
			CopyFrom(target->ranges, left->ranges);
			// TODO: (enumerable) foreach
			for (vint i = 0; i < right->ranges.Count(); i++)
			{
				if (!target->AddRangeWithConflict(right->ranges[i]))
				{
					CHECK_ERROR(false, L"RegexNode::operator%(const RegexNode&)#Failed to create charset expression from operator %.");
				}
			}
			return RegexNode(target);
		}

/***********************************************************************
Regex Writer
***********************************************************************/

		RegexNode rCapture(const U32String& name, const RegexNode& node)
		{
			auto target = Ptr(new CaptureExpression);
			target->name = name;
			target->expression = node.expression;
			return RegexNode(target);
		}

		RegexNode rUsing(const U32String& name)
		{
			auto target = Ptr(new UsingExpression);
			target->name = name;
			return RegexNode(target);
		}

		RegexNode rMatch(const U32String& name, vint index)
		{
			auto target = Ptr(new MatchExpression);
			target->name = name;
			target->index = index;
			return RegexNode(target);
		}

		RegexNode rMatch(vint index)
		{
			auto target = Ptr(new MatchExpression);
			target->index = index;
			return RegexNode(target);
		}

		RegexNode rBegin()
		{
			return RegexNode(Ptr(new BeginExpression));
		}

		RegexNode rEnd()
		{
			return RegexNode(Ptr(new EndExpression));
		}

		RegexNode rC(char32_t a, char32_t b)
		{
			if (!b)b = a;
			auto target = Ptr(new CharSetExpression);
			target->reverse = false;
			target->AddRangeWithConflict(CharRange(a, b));
			return RegexNode(target);
		}

		RegexNode r_d()
		{
			return rC(U'0', U'9');
		}

		RegexNode r_l()
		{
			return rC(U'a', U'z') % rC(U'A', U'Z') % rC(U'_');
		}

		RegexNode r_w()
		{
			return rC(U'0', U'9') % rC(U'a', U'z') % rC(U'A', U'Z') % rC(U'_');
		}

		RegexNode rAnyChar()
		{
			return rC(1, MaxChar32);
		}
	}
}