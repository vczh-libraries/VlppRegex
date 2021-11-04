/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_REGEX_REGEXWRITER
#define VCZH_REGEX_REGEXWRITER

#include "RegexExpression.h"

namespace vl
{
	namespace regex
	{
		class RegexNode : public Object
		{
		public:
			vl::regex_internal::Expression::Ref		expression;

			RegexNode(vl::regex_internal::Expression::Ref _expression);

			RegexNode					Some()const;
			RegexNode					Any()const;
			RegexNode					Opt()const;
			RegexNode					Loop(vint min, vint max)const;
			RegexNode					AtLeast(vint min)const;
			RegexNode					operator+(const RegexNode& node)const;
			RegexNode					operator|(const RegexNode& node)const;
			RegexNode					operator+()const;
			RegexNode					operator-()const;
			RegexNode					operator!()const;
			RegexNode					operator%(const RegexNode& node)const;
		};

		extern RegexNode				rCapture(const U32String& name, const RegexNode& node);
		extern RegexNode				rUsing(const U32String& name);
		extern RegexNode				rMatch(const U32String& name, vint index=-1);
		extern RegexNode				rMatch(vint index);
		extern RegexNode				rBegin();
		extern RegexNode				rEnd();
		extern RegexNode				rC(char32_t a, char32_t b=0);
		extern RegexNode				r_d();
		extern RegexNode				r_l();
		extern RegexNode				r_w();
		extern RegexNode				rAnyChar();
	}
}

#endif