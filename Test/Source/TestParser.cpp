#include <stdlib.h>
#include "../../Source/Regex/RegexExpression.h"
#include "../../Source/Regex/RegexWriter.h"
#include "../../Source/Regex/RegexPure.h"
#include "../../Source/Regex/RegexRich.h"
#include "../../Source/Regex/Regex.h"
#include "../../Import/VlppOS.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::regex;
using namespace vl::regex_internal;
using namespace vl::stream;

extern WString GetTestResourcePath();
extern WString GetTestOutputPath();

TEST_FILE
{
	void RegexAssert(const wchar_t* input, RegexNode node, bool pure)
	{
		Expression::Ref exp = ParseExpression(input);
		TEST_ASSERT(exp->IsEqual(node.expression.Obj()));
		TEST_ASSERT(exp->CanTreatAsPure() == pure);
	}

	TEST_CASE(TestRegexCharSetParsing)
	{
		RegexAssert(L"a", rC(L'a'), true);
		RegexAssert(L"vczh", rC(L'v') + rC(L'c') + rC(L'z') + rC(L'h'), true);

		RegexAssert(L"/d", r_d(), true);
		RegexAssert(L"/l", r_l(), true);
		RegexAssert(L"/w", r_w(), true);
		RegexAssert(L"/D", !r_d(), true);
		RegexAssert(L"/L", !r_l(), true);
		RegexAssert(L"/W", !r_w(), true);
		RegexAssert(L"/r", rC(L'\r'), true);
		RegexAssert(L"/n", rC(L'\n'), true);
		RegexAssert(L"/t", rC(L'\t'), true);

		RegexAssert(L"\\d", r_d(), true);
		RegexAssert(L"\\l", r_l(), true);
		RegexAssert(L"\\w", r_w(), true);
		RegexAssert(L"\\D", !r_d(), true);
		RegexAssert(L"\\L", !r_l(), true);
		RegexAssert(L"\\W", !r_w(), true);
		RegexAssert(L"\\r", rC(L'\r'), true);
		RegexAssert(L"\\n", rC(L'\n'), true);
		RegexAssert(L"\\t", rC(L'\t'), true);

		RegexAssert(L"^", rBegin(), false);
		RegexAssert(L"$", rEnd(), false);
		RegexAssert(L"[abc]", rC(L'a') % rC(L'b') % rC(L'c'), true);
		RegexAssert(L"[0-9]", r_d(), true);
		RegexAssert(L"[a-zA-Z_]", r_l(), true);
		RegexAssert(L"[a-zA-Z0-9_]", r_w(), true);
		RegexAssert(L"[^0-9]", !r_d(), true);
		RegexAssert(L"[^a-zA-Z_]", !r_l(), true);
		RegexAssert(L"[^a-zA-Z0-9_]", !r_w(), true);
	}

	TEST_CASE(TestRegexLoopParsing)
	{
		RegexAssert(L"/d+", r_d().Some(), true);
		RegexAssert(L"/d*", r_d().Any(), true);
		RegexAssert(L"/d?", r_d().Opt(), true);
		RegexAssert(L"/d{3}", r_d().Loop(3, 3), true);
		RegexAssert(L"/d{3,5}", r_d().Loop(3, 5), true);
		RegexAssert(L"/d{4,}", r_d().AtLeast(4), true);

		RegexAssert(L"\\d+", r_d().Some(), true);
		RegexAssert(L"\\d*", r_d().Any(), true);
		RegexAssert(L"\\d?", r_d().Opt(), true);
		RegexAssert(L"\\d{3}", r_d().Loop(3, 3), true);
		RegexAssert(L"\\d{3,5}", r_d().Loop(3, 5), true);
		RegexAssert(L"\\d{4,}", r_d().AtLeast(4), true);
	}

	TEST_CASE(TestRegexFunctionParsing)
	{
		RegexAssert(L"(<name>/d+)", rCapture(L"name", r_d().Some()), true);
		RegexAssert(L"(<&name>)", rUsing(L"name"), false);
		RegexAssert(L"(<$name>)", rMatch(L"name"), false);
		RegexAssert(L"(<$name;3>)", rMatch(L"name", 3), false);
		RegexAssert(L"(<$3>)", rMatch(3), false);
		RegexAssert(L"(=/d+)", +r_d().Some(), false);
		RegexAssert(L"(!/d+)", -r_d().Some(), false);
		RegexAssert(L"(=\\d+)", +r_d().Some(), false);
		RegexAssert(L"(!\\d+)", -r_d().Some(), false);
	}

	TEST_CASE(TestRegexComplexParsing)
	{
		RegexAssert(L"a+(bc)*", rC(L'a').Some() + (rC(L'b') + rC(L'c')).Any(), true);
		RegexAssert(L"(1+2)*(3+4)", (rC(L'1').Some() + rC(L'2')).Any() + (rC(L'3').Some() + rC(L'4')), true);
		RegexAssert(L"[a-zA-Z_][a-zA-Z0-9_]*", r_l() + r_w().Any(), true);
		RegexAssert(L"((<part>/d+).){3}(<part>/d+)", (rCapture(L"part", r_d().Some()) + rC(L'.')).Loop(3, 3) + rCapture(L"part", r_d().Some()), true);
		RegexAssert(L"ab|ac", (rC(L'a') + rC(L'b')) | (rC(L'a') + rC(L'c')), true);
		RegexAssert(L"a(b|c)", rC(L'a') + (rC(L'b') | rC(L'c')), true);
		RegexAssert(L"/.*[/r/n/t]", rAnyChar().Any() + (rC(L'\r') % rC(L'\n') % rC(L'\t')), true);

		RegexAssert(L"((<part>\\d+).){3}(<part>\\d+)", (rCapture(L"part", r_d().Some()) + rC(L'.')).Loop(3, 3) + rCapture(L"part", r_d().Some()), true);
		RegexAssert(L"\\.*[\\r\\n\\t]", rAnyChar().Any() + (rC(L'\r') % rC(L'\n') % rC(L'\t')), true);
	}

	TEST_CASE(TestRegexCompleteParsingA)
	{
		WString code = L"(<#part>/d+)(<#capture>(<section>(<&part>)))((<&capture>).){3}(<&capture>)";
		RegexExpression::Ref regex = ParseRegexExpression(code);

		Expression::Ref part = r_d().Some().expression;
		Expression::Ref capture = rCapture(L"section", rUsing(L"part")).expression;
		Expression::Ref main = ((rUsing(L"capture") + rC(L'.')).Loop(3, 3) + rUsing(L"capture")).expression;

		TEST_ASSERT(regex->definitions.Count() == 2);
		TEST_ASSERT(regex->definitions.Keys()[0] == L"capture");
		TEST_ASSERT(regex->definitions.Keys()[1] == L"part");

		TEST_ASSERT(regex->definitions[L"part"]->IsEqual(part.Obj()));
		TEST_ASSERT(regex->definitions[L"capture"]->IsEqual(capture.Obj()));
		TEST_ASSERT(regex->expression->IsEqual(main.Obj()));
	}

	TEST_CASE(TestRegexCompleteParsingB)
	{
		WString code = L"((<part>\\d+).){3}(<part>\\d+)";
		RegexExpression::Ref regex = ParseRegexExpression(code);

		Expression::Ref main = ((rCapture(L"part", r_d().Some()) + rC(L'.')).Loop(3, 3) + rCapture(L"part", r_d().Some())).expression;

		TEST_ASSERT(regex->definitions.Count() == 0);
		TEST_ASSERT(regex->expression->IsEqual(main.Obj()));
	}
}