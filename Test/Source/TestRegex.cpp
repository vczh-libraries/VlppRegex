#include "../../Source/Regex/AST/RegexWriter.h"
#include "../../Source/Regex/Regex.h"

using namespace vl;
using namespace vl::regex;
using namespace vl::regex_internal;

void NormalizedRegexAssert(const char32_t* input, RegexNode node)
{
	CharRange::List subsets;
	Expression::Ref exp = ParseExpression(input);
	exp->NormalizeCharSet(subsets);
	TEST_ASSERT(exp->IsEqual(node.expression.Obj()));

	subsets.Clear();
	exp->CollectCharSet(subsets);
	exp->ApplyCharSet(subsets);
	TEST_ASSERT(exp->IsEqual(node.expression.Obj()));
}

void MergedRegexAssert(const char32_t* input, RegexNode node)
{
	RegexExpression::Ref regex = ParseRegexExpression(input);
	Expression::Ref exp = regex->Merge();
	TEST_ASSERT(exp->IsEqual(node.expression.Obj()));
}

TEST_FILE
{
	TEST_CASE(L"Test charset normalization")
	{
		NormalizedRegexAssert(U"[a-g][h-n]", rC(U'a', U'g') + rC(U'h', U'n'));
		NormalizedRegexAssert(U"[a-g][g-n]", (rC(U'a', U'f') % rC(U'g')) + (rC(U'g') % rC(U'h', U'n')));
		NormalizedRegexAssert(U"/w+vczh", (
			rC(U'0', U'9') % rC(U'A', U'Z') % rC(U'_') % rC(U'a', U'b') % rC(U'c') % rC(U'd', U'g') % rC(U'h') % rC(U'i', U'u') % rC(U'v') % rC(U'w', U'y') % rC(U'z')
			).Some() + rC(U'v') + rC(U'c') + rC(U'z') + rC(U'h'));
		NormalizedRegexAssert(U"[0-2][1-3][2-4]", (rC(U'0') % rC(U'1') % rC(U'2')) + (rC(U'1') % rC(U'2') % rC(U'3')) + (rC(U'2') % rC(U'3') % rC(U'4')));
		NormalizedRegexAssert(U"[^C-X][A-Z]", (rC(1, U'A' - 1) % rC(U'A', U'B') % rC(U'Y', U'Z') % rC(U'Z' + 1, 65535)) + (rC(U'A', U'B') % rC(U'C', U'X') % rC(U'Y', U'Z')));
	});

	TEST_CASE(L"Test expression merging")
	{
		const char32_t* code = U"(<#part>/d+)(<#capture>(<section>(<&part>)))((<&capture>).){3}(<&capture>)";
		RegexNode node = (rCapture(U"section", r_d().Some()) + rC(U'.')).Loop(3, 3) + rCapture(U"section", r_d().Some());
		MergedRegexAssert(code, node);
	});

	auto TestRegexMatchPosition = [](bool preferPure)
	{
		Regex regex(U"/d+", preferPure);
		TEST_ASSERT(regex.IsPureMatch() == preferPure);
		TEST_ASSERT(regex.IsPureTest() == preferPure);
		TEST_ASSERT(regex.TestHead(U"123abc") == true);
		TEST_ASSERT(regex.TestHead(U"abc123abc") == false);
		TEST_ASSERT(regex.TestHead(U"abc") == false);
		TEST_ASSERT(regex.Test(U"123abc") == true);
		TEST_ASSERT(regex.Test(U"abc123abc") == true);
		TEST_ASSERT(regex.Test(U"abc") == false);

		RegexMatch::Ref match;
		RegexMatch::List matches;

		match = regex.MatchHead(U"123abc");
		TEST_ASSERT(match);
		TEST_ASSERT(match->Success() == true);
		TEST_ASSERT(match->Result().Start() == 0);
		TEST_ASSERT(match->Result().Length() == 3);
		TEST_ASSERT(match->Result().Value() == U"123");

		match = regex.MatchHead(U"abc123abc");
		TEST_ASSERT(!match);

		match = regex.Match(U"123abc");
		TEST_ASSERT(match);
		TEST_ASSERT(match->Success() == true);
		TEST_ASSERT(match->Result().Start() == 0);
		TEST_ASSERT(match->Result().Length() == 3);
		TEST_ASSERT(match->Result().Value() == U"123");

		match = regex.Match(U"abc123abc");
		TEST_ASSERT(match);
		TEST_ASSERT(match->Success() == true);
		TEST_ASSERT(match->Result().Start() == 3);
		TEST_ASSERT(match->Result().Length() == 3);
		TEST_ASSERT(match->Result().Value() == U"123");

		matches.Clear();
		regex.Search(U"12abc34def56", matches);
		TEST_ASSERT(matches.Count() == 3);
		TEST_ASSERT(matches[0]->Success() == true);
		TEST_ASSERT(matches[0]->Result().Start() == 0);
		TEST_ASSERT(matches[0]->Result().Length() == 2);
		TEST_ASSERT(matches[0]->Result().Value() == U"12");
		TEST_ASSERT(matches[1]->Success() == true);
		TEST_ASSERT(matches[1]->Result().Start() == 5);
		TEST_ASSERT(matches[1]->Result().Length() == 2);
		TEST_ASSERT(matches[1]->Result().Value() == U"34");
		TEST_ASSERT(matches[2]->Success() == true);
		TEST_ASSERT(matches[2]->Result().Start() == 10);
		TEST_ASSERT(matches[2]->Result().Length() == 2);
		TEST_ASSERT(matches[2]->Result().Value() == U"56");

		matches.Clear();
		regex.Split(U"12abc34def56", false, matches);
		TEST_ASSERT(matches.Count() == 2);
		TEST_ASSERT(matches[0]->Success() == false);
		TEST_ASSERT(matches[0]->Result().Start() == 2);
		TEST_ASSERT(matches[0]->Result().Length() == 3);
		TEST_ASSERT(matches[0]->Result().Value() == U"abc");
		TEST_ASSERT(matches[1]->Success() == false);
		TEST_ASSERT(matches[1]->Result().Start() == 7);
		TEST_ASSERT(matches[1]->Result().Length() == 3);
		TEST_ASSERT(matches[1]->Result().Value() == U"def");

		matches.Clear();
		regex.Split(U"12abc34def56", true, matches);
		TEST_ASSERT(matches.Count() == 4);
		TEST_ASSERT(matches[0]->Success() == false);
		TEST_ASSERT(matches[0]->Result().Start() == 0);
		TEST_ASSERT(matches[0]->Result().Length() == 0);
		TEST_ASSERT(matches[0]->Result().Value() == U"");
		TEST_ASSERT(matches[1]->Success() == false);
		TEST_ASSERT(matches[1]->Result().Start() == 2);
		TEST_ASSERT(matches[1]->Result().Length() == 3);
		TEST_ASSERT(matches[1]->Result().Value() == U"abc");
		TEST_ASSERT(matches[2]->Success() == false);
		TEST_ASSERT(matches[2]->Result().Start() == 7);
		TEST_ASSERT(matches[2]->Result().Length() == 3);
		TEST_ASSERT(matches[2]->Result().Value() == U"def");
		TEST_ASSERT(matches[3]->Success() == false);
		TEST_ASSERT(matches[3]->Result().Start() == 12);
		TEST_ASSERT(matches[3]->Result().Length() == 0);
		TEST_ASSERT(matches[3]->Result().Value() == U"");

		matches.Clear();
		regex.Cut(U"12abc34def56", false, matches);
		TEST_ASSERT(matches.Count() == 5);
		TEST_ASSERT(matches[0]->Success() == true);
		TEST_ASSERT(matches[0]->Result().Start() == 0);
		TEST_ASSERT(matches[0]->Result().Length() == 2);
		TEST_ASSERT(matches[0]->Result().Value() == U"12");
		TEST_ASSERT(matches[1]->Success() == false);
		TEST_ASSERT(matches[1]->Result().Start() == 2);
		TEST_ASSERT(matches[1]->Result().Length() == 3);
		TEST_ASSERT(matches[1]->Result().Value() == U"abc");
		TEST_ASSERT(matches[2]->Success() == true);
		TEST_ASSERT(matches[2]->Result().Start() == 5);
		TEST_ASSERT(matches[2]->Result().Length() == 2);
		TEST_ASSERT(matches[2]->Result().Value() == U"34");
		TEST_ASSERT(matches[3]->Success() == false);
		TEST_ASSERT(matches[3]->Result().Start() == 7);
		TEST_ASSERT(matches[3]->Result().Length() == 3);
		TEST_ASSERT(matches[3]->Result().Value() == U"def");
		TEST_ASSERT(matches[4]->Success() == true);
		TEST_ASSERT(matches[4]->Result().Start() == 10);
		TEST_ASSERT(matches[4]->Result().Length() == 2);
		TEST_ASSERT(matches[4]->Result().Value() == U"56");

		matches.Clear();
		regex.Cut(U"12abc34def56", true, matches);
		TEST_ASSERT(matches.Count() == 7);
		TEST_ASSERT(matches[0]->Success() == false);
		TEST_ASSERT(matches[0]->Result().Start() == 0);
		TEST_ASSERT(matches[0]->Result().Length() == 0);
		TEST_ASSERT(matches[0]->Result().Value() == U"");
		TEST_ASSERT(matches[1]->Success() == true);
		TEST_ASSERT(matches[1]->Result().Start() == 0);
		TEST_ASSERT(matches[1]->Result().Length() == 2);
		TEST_ASSERT(matches[1]->Result().Value() == U"12");
		TEST_ASSERT(matches[2]->Success() == false);
		TEST_ASSERT(matches[2]->Result().Start() == 2);
		TEST_ASSERT(matches[2]->Result().Length() == 3);
		TEST_ASSERT(matches[2]->Result().Value() == U"abc");
		TEST_ASSERT(matches[3]->Success() == true);
		TEST_ASSERT(matches[3]->Result().Start() == 5);
		TEST_ASSERT(matches[3]->Result().Length() == 2);
		TEST_ASSERT(matches[3]->Result().Value() == U"34");
		TEST_ASSERT(matches[4]->Success() == false);
		TEST_ASSERT(matches[4]->Result().Start() == 7);
		TEST_ASSERT(matches[4]->Result().Length() == 3);
		TEST_ASSERT(matches[4]->Result().Value() == U"def");
		TEST_ASSERT(matches[5]->Success() == true);
		TEST_ASSERT(matches[5]->Result().Start() == 10);
		TEST_ASSERT(matches[5]->Result().Length() == 2);
		TEST_ASSERT(matches[5]->Result().Value() == U"56");
		TEST_ASSERT(matches[6]->Success() == false);
		TEST_ASSERT(matches[6]->Result().Start() == 12);
		TEST_ASSERT(matches[6]->Result().Length() == 0);
		TEST_ASSERT(matches[6]->Result().Value() == U"");

		matches.Clear();
		regex.Cut(U"XY12abc34def56ZW", true, matches);
		TEST_ASSERT(matches.Count() == 7);
		TEST_ASSERT(matches[0]->Success() == false);
		TEST_ASSERT(matches[0]->Result().Start() == 0);
		TEST_ASSERT(matches[0]->Result().Length() == 2);
		TEST_ASSERT(matches[0]->Result().Value() == U"XY");
		TEST_ASSERT(matches[1]->Success() == true);
		TEST_ASSERT(matches[1]->Result().Start() == 2);
		TEST_ASSERT(matches[1]->Result().Length() == 2);
		TEST_ASSERT(matches[1]->Result().Value() == U"12");
		TEST_ASSERT(matches[2]->Success() == false);
		TEST_ASSERT(matches[2]->Result().Start() == 4);
		TEST_ASSERT(matches[2]->Result().Length() == 3);
		TEST_ASSERT(matches[2]->Result().Value() == U"abc");
		TEST_ASSERT(matches[3]->Success() == true);
		TEST_ASSERT(matches[3]->Result().Start() == 7);
		TEST_ASSERT(matches[3]->Result().Length() == 2);
		TEST_ASSERT(matches[3]->Result().Value() == U"34");
		TEST_ASSERT(matches[4]->Success() == false);
		TEST_ASSERT(matches[4]->Result().Start() == 9);
		TEST_ASSERT(matches[4]->Result().Length() == 3);
		TEST_ASSERT(matches[4]->Result().Value() == U"def");
		TEST_ASSERT(matches[5]->Success() == true);
		TEST_ASSERT(matches[5]->Result().Start() == 12);
		TEST_ASSERT(matches[5]->Result().Length() == 2);
		TEST_ASSERT(matches[5]->Result().Value() == U"56");
		TEST_ASSERT(matches[6]->Success() == false);
		TEST_ASSERT(matches[6]->Result().Start() == 14);
		TEST_ASSERT(matches[6]->Result().Length() == 2);
		TEST_ASSERT(matches[6]->Result().Value() == U"ZW");
	};

	TEST_CASE(L"Test matching position")
	{
		TestRegexMatchPosition(true);
		TestRegexMatchPosition(false);
	});

	TEST_CASE(L"Test capturing")
	{
		{
			Regex regex(U"^(<a>/w+?)(<b>/w+?)((<$a>)(<$b>))+(<$a>)/w{6}$", true);
			TEST_ASSERT(regex.IsPureMatch() == false);
			TEST_ASSERT(regex.IsPureTest() == false);

			RegexMatch::Ref match = regex.Match(U"vczhgeniusvczhgeniusvczhgeniusvczhgenius");
			TEST_ASSERT(match);
			TEST_ASSERT(match->Success() == true);
			TEST_ASSERT(match->Result().Start() == 0);
			TEST_ASSERT(match->Result().Length() == 40);
			TEST_ASSERT(match->Result().Value() == U"vczhgeniusvczhgeniusvczhgeniusvczhgenius");
			TEST_ASSERT(match->Groups().Keys().Count() == 2);
			TEST_ASSERT(match->Groups().Keys()[0] == U"a");
			TEST_ASSERT(match->Groups().Keys()[1] == U"b");
			TEST_ASSERT(match->Groups()[U"a"].Count() == 1);
			TEST_ASSERT(match->Groups()[U"a"].Get(0).Start() == 0);
			TEST_ASSERT(match->Groups()[U"a"].Get(0).Length() == 4);
			TEST_ASSERT(match->Groups()[U"a"].Get(0).Value() == U"vczh");
			TEST_ASSERT(match->Groups()[U"b"].Count() == 1);
			TEST_ASSERT(match->Groups()[U"b"].Get(0).Start() == 4);
			TEST_ASSERT(match->Groups()[U"b"].Get(0).Length() == 6);
			TEST_ASSERT(match->Groups()[U"b"].Get(0).Value() == U"genius");
		}
		{
			Regex regex(U"^(?/d+).(?/d+).(?/d+).(<$0>).(<$1>).(<$2>)$");
			RegexMatch::Ref match;

			match = regex.MatchHead(U"12.34.56.12.56.34");
			TEST_ASSERT(!match);

			match = regex.MatchHead(U"123.4.56.123.4.56");
			TEST_ASSERT(match);
			TEST_ASSERT(match->Success() == true);
			TEST_ASSERT(match->Result().Start() == 0);
			TEST_ASSERT(match->Result().Length() == 17);
			TEST_ASSERT(match->Result().Value() == U"123.4.56.123.4.56");
			TEST_ASSERT(match->Captures().Count() == 3);
			TEST_ASSERT(match->Captures().Get(0).Start() == 0);
			TEST_ASSERT(match->Captures().Get(0).Length() == 3);
			TEST_ASSERT(match->Captures().Get(0).Value() == U"123");
			TEST_ASSERT(match->Captures().Get(1).Start() == 4);
			TEST_ASSERT(match->Captures().Get(1).Length() == 1);
			TEST_ASSERT(match->Captures().Get(1).Value() == U"4");
			TEST_ASSERT(match->Captures().Get(2).Start() == 6);
			TEST_ASSERT(match->Captures().Get(2).Length() == 2);
			TEST_ASSERT(match->Captures().Get(2).Value() == U"56");
		}
	});

#ifdef NDEBUG
	auto FindRows = [](U32String* lines, int count, const U32String& pattern)
	{
		Regex regex(pattern);
		DateTime dt1 = DateTime::LocalTime();
		for (int i = 0; i < 10000000; i++)
		{
			for (int j = 0; j < count; j++)
			{
				bool result = regex.TestHead(lines[j]);
				TEST_ASSERT(result);
			}
		}
		DateTime dt2 = DateTime::LocalTime();
		vuint64_t ms = dt2.totalMilliseconds - dt1.totalMilliseconds;
		unittest::UnitTest::PrintMessage(L"Running 10000000 times of Regex::TestHead uses: " + i64tow(ms) + L" milliseconds.", unittest::UnitTest::MessageKind::Info);
	};

	TEST_CASE(L"Test performance")
	{
		U32String pattern = U"(\\.*A\\.*B\\.*C|\\.*A\\.*C\\.*B|\\.*B\\.*A\\.*C|\\.*B\\.*C\\.*A|\\.*C\\.*A\\.*B|\\.*C\\.*B\\.*A)";
		U32String lines[] =
		{
			U"XAYBZC",
			U"XAYCZB",
			U"XBYAZC",
			U"XBYCZA",
			U"XCYAZB",
			U"XCYBZA",
		};
		FindRows(lines, sizeof(lines) / sizeof(*lines), pattern);
		FindRows(lines, sizeof(lines) / sizeof(*lines), pattern);
	});
#endif
}