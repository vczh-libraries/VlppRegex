#include "../../Source/Regex/AST/RegexWriter.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::regex;
using namespace vl::regex_internal;

TEST_FILE
{
	auto RegexAssert = [](const char32_t* input, RegexNode node, bool pure)
	{
		TEST_CASE(u32tow(input))
		{
			Ptr<Expression> exp = ParseExpression(input);
			TEST_ASSERT(exp->IsEqual(node.expression.Obj()));
			TEST_ASSERT(exp->CanTreatAsPure() == pure);
		});
	};

	TEST_CATEGORY(L"Parser: charset")
	{
		RegexAssert(U"a", rC(U'a'), true);
		RegexAssert(U"vczh", rC(U'v') + rC(U'c') + rC(U'z') + rC(U'h'), true);

		RegexAssert(U"/d", r_d(), true);
		RegexAssert(U"/l", r_l(), true);
		RegexAssert(U"/w", r_w(), true);
		RegexAssert(U"/D", !r_d(), true);
		RegexAssert(U"/L", !r_l(), true);
		RegexAssert(U"/W", !r_w(), true);
		RegexAssert(U"/r", rC(L'\r'), true);
		RegexAssert(U"/n", rC(L'\n'), true);
		RegexAssert(U"/t", rC(L'\t'), true);

		RegexAssert(U"\\d", r_d(), true);
		RegexAssert(U"\\l", r_l(), true);
		RegexAssert(U"\\w", r_w(), true);
		RegexAssert(U"\\D", !r_d(), true);
		RegexAssert(U"\\L", !r_l(), true);
		RegexAssert(U"\\W", !r_w(), true);
		RegexAssert(U"\\r", rC(L'\r'), true);
		RegexAssert(U"\\n", rC(L'\n'), true);
		RegexAssert(U"\\t", rC(L'\t'), true);

		RegexAssert(U"^", rBegin(), false);
		RegexAssert(U"$", rEnd(), false);
		RegexAssert(U"[abc]", rC(U'a') % rC(U'b') % rC(U'c'), true);
		RegexAssert(U"[0-9]", r_d(), true);
		RegexAssert(U"[a-zA-Z_]", r_l(), true);
		RegexAssert(U"[a-zA-Z0-9_]", r_w(), true);
		RegexAssert(U"[^0-9]", !r_d(), true);
		RegexAssert(U"[^a-zA-Z_]", !r_l(), true);
		RegexAssert(U"[^a-zA-Z0-9_]", !r_w(), true);
	});

	TEST_CATEGORY(L"Parser: looping")
	{
		RegexAssert(U"/d+", r_d().Some(), true);
		RegexAssert(U"/d*", r_d().Any(), true);
		RegexAssert(U"/d?", r_d().Opt(), true);
		RegexAssert(U"/d{3}", r_d().Loop(3, 3), true);
		RegexAssert(U"/d{3,5}", r_d().Loop(3, 5), true);
		RegexAssert(U"/d{4,}", r_d().AtLeast(4), true);

		RegexAssert(U"\\d+", r_d().Some(), true);
		RegexAssert(U"\\d*", r_d().Any(), true);
		RegexAssert(U"\\d?", r_d().Opt(), true);
		RegexAssert(U"\\d{3}", r_d().Loop(3, 3), true);
		RegexAssert(U"\\d{3,5}", r_d().Loop(3, 5), true);
		RegexAssert(U"\\d{4,}", r_d().AtLeast(4), true);
	});

	TEST_CATEGORY(L"Parser: function")
	{
		RegexAssert(U"(<name>/d+)", rCapture(U"name", r_d().Some()), true);
		RegexAssert(U"(<&name>)", rUsing(U"name"), false);
		RegexAssert(U"(<$name>)", rMatch(U"name"), false);
		RegexAssert(U"(<$name;3>)", rMatch(U"name", 3), false);
		RegexAssert(U"(<$3>)", rMatch(3), false);
		RegexAssert(U"(=/d+)", +r_d().Some(), false);
		RegexAssert(U"(!/d+)", -r_d().Some(), false);
		RegexAssert(U"(=\\d+)", +r_d().Some(), false);
		RegexAssert(U"(!\\d+)", -r_d().Some(), false);
	});

	TEST_CATEGORY(L"Parser: complex string")
	{
		RegexAssert(U"a+(bc)*", rC(U'a').Some() + (rC(U'b') + rC(U'c')).Any(), true);
		RegexAssert(U"(1+2)*(3+4)", (rC(U'1').Some() + rC(U'2')).Any() + (rC(U'3').Some() + rC(U'4')), true);
		RegexAssert(U"[a-zA-Z_][a-zA-Z0-9_]*", r_l() + r_w().Any(), true);
		RegexAssert(U"((<part>/d+).){3}(<part>/d+)", (rCapture(U"part", r_d().Some()) + rC(U'.')).Loop(3, 3) + rCapture(U"part", r_d().Some()), true);
		RegexAssert(U"ab|ac", (rC(U'a') + rC(U'b')) | (rC(U'a') + rC(U'c')), true);
		RegexAssert(U"a(b|c)", rC(U'a') + (rC(U'b') | rC(U'c')), true);
		RegexAssert(U"/.*[/r/n/t]", rAnyChar().Any() + (rC(L'\r') % rC(L'\n') % rC(L'\t')), true);

		RegexAssert(U"((<part>\\d+).){3}(<part>\\d+)", (rCapture(U"part", r_d().Some()) + rC(U'.')).Loop(3, 3) + rCapture(U"part", r_d().Some()), true);
		RegexAssert(U"\\.*[\\r\\n\\t]", rAnyChar().Any() + (rC(L'\r') % rC(L'\n') % rC(L'\t')), true);
	});

	TEST_CASE(L"Parser: integration 1")
	{
		U32String code = U"(<#part>/d+)(<#capture>(<section>(<&part>)))((<&capture>).){3}(<&capture>)";
		auto regex = ParseRegexExpression(code);

		auto part = r_d().Some().expression;
		auto capture = rCapture(U"section", rUsing(U"part")).expression;
		auto main = ((rUsing(U"capture") + rC(U'.')).Loop(3, 3) + rUsing(U"capture")).expression;

		TEST_ASSERT(regex->definitions.Count() == 2);
		TEST_ASSERT(regex->definitions.Keys()[0] == U"capture");
		TEST_ASSERT(regex->definitions.Keys()[1] == U"part");

		TEST_ASSERT(regex->definitions[U"part"]->IsEqual(part.Obj()));
		TEST_ASSERT(regex->definitions[U"capture"]->IsEqual(capture.Obj()));
		TEST_ASSERT(regex->expression->IsEqual(main.Obj()));
	});

	TEST_CASE(L"Parser: integration 2")
	{
		U32String code = U"((<part>\\d+).){3}(<part>\\d+)";
		auto regex = ParseRegexExpression(code);

		auto main = ((rCapture(U"part", r_d().Some()) + rC(U'.')).Loop(3, 3) + rCapture(U"part", r_d().Some())).expression;

		TEST_ASSERT(regex->definitions.Count() == 0);
		TEST_ASSERT(regex->expression->IsEqual(main.Obj()));
	});
}