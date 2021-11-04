#include "../../Source/Regex/AST/RegexWriter.h"
#include "../../Source/Regex/Regex.h"

using namespace vl;
using namespace vl::regex;
using namespace vl::regex_internal;

void NormalizedRegexAssert(const wchar_t* input, RegexNode node)
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

void MergedRegexAssert(const wchar_t* input, RegexNode node)
{
	RegexExpression::Ref regex = ParseRegexExpression(input);
	Expression::Ref exp = regex->Merge();
	TEST_ASSERT(exp->IsEqual(node.expression.Obj()));
}

TEST_FILE
{
	TEST_CASE(L"Test charset normalization")
	{
		NormalizedRegexAssert(L"[a-g][h-n]", rC(L'a', L'g') + rC(L'h', L'n'));
		NormalizedRegexAssert(L"[a-g][g-n]", (rC(L'a', L'f') % rC(L'g')) + (rC(L'g') % rC(L'h', L'n')));
		NormalizedRegexAssert(L"/w+vczh", (
			rC(L'0', L'9') % rC(L'A', L'Z') % rC(L'_') % rC(L'a', L'b') % rC(L'c') % rC(L'd', L'g') % rC(L'h') % rC(L'i', L'u') % rC(L'v') % rC(L'w', L'y') % rC(L'z')
			).Some() + rC(L'v') + rC(L'c') + rC(L'z') + rC(L'h'));
		NormalizedRegexAssert(L"[0-2][1-3][2-4]", (rC(L'0') % rC(L'1') % rC(L'2')) + (rC(L'1') % rC(L'2') % rC(L'3')) + (rC(L'2') % rC(L'3') % rC(L'4')));
		NormalizedRegexAssert(L"[^C-X][A-Z]", (rC(1, L'A' - 1) % rC(L'A', L'B') % rC(L'Y', L'Z') % rC(L'Z' + 1, 65535)) + (rC(L'A', L'B') % rC(L'C', L'X') % rC(L'Y', L'Z')));
	});

	TEST_CASE(L"Test expression merging")
	{
		const wchar_t* code = L"(<#part>/d+)(<#capture>(<section>(<&part>)))((<&capture>).){3}(<&capture>)";
		RegexNode node = (rCapture(L"section", r_d().Some()) + rC(L'.')).Loop(3, 3) + rCapture(L"section", r_d().Some());
		MergedRegexAssert(code, node);
	});

	auto TestRegexMatchPosition = [](bool preferPure)
	{
		Regex regex(L"/d+", preferPure);
		TEST_ASSERT(regex.IsPureMatch() == preferPure);
		TEST_ASSERT(regex.IsPureTest() == preferPure);
		TEST_ASSERT(regex.TestHead(L"123abc") == true);
		TEST_ASSERT(regex.TestHead(L"abc123abc") == false);
		TEST_ASSERT(regex.TestHead(L"abc") == false);
		TEST_ASSERT(regex.Test(L"123abc") == true);
		TEST_ASSERT(regex.Test(L"abc123abc") == true);
		TEST_ASSERT(regex.Test(L"abc") == false);

		RegexMatch::Ref match;
		RegexMatch::List matches;

		match = regex.MatchHead(L"123abc");
		TEST_ASSERT(match);
		TEST_ASSERT(match->Success() == true);
		TEST_ASSERT(match->Result().Start() == 0);
		TEST_ASSERT(match->Result().Length() == 3);
		TEST_ASSERT(match->Result().Value() == L"123");

		match = regex.MatchHead(L"abc123abc");
		TEST_ASSERT(!match);

		match = regex.Match(L"123abc");
		TEST_ASSERT(match);
		TEST_ASSERT(match->Success() == true);
		TEST_ASSERT(match->Result().Start() == 0);
		TEST_ASSERT(match->Result().Length() == 3);
		TEST_ASSERT(match->Result().Value() == L"123");

		match = regex.Match(L"abc123abc");
		TEST_ASSERT(match);
		TEST_ASSERT(match->Success() == true);
		TEST_ASSERT(match->Result().Start() == 3);
		TEST_ASSERT(match->Result().Length() == 3);
		TEST_ASSERT(match->Result().Value() == L"123");

		matches.Clear();
		regex.Search(L"12abc34def56", matches);
		TEST_ASSERT(matches.Count() == 3);
		TEST_ASSERT(matches[0]->Success() == true);
		TEST_ASSERT(matches[0]->Result().Start() == 0);
		TEST_ASSERT(matches[0]->Result().Length() == 2);
		TEST_ASSERT(matches[0]->Result().Value() == L"12");
		TEST_ASSERT(matches[1]->Success() == true);
		TEST_ASSERT(matches[1]->Result().Start() == 5);
		TEST_ASSERT(matches[1]->Result().Length() == 2);
		TEST_ASSERT(matches[1]->Result().Value() == L"34");
		TEST_ASSERT(matches[2]->Success() == true);
		TEST_ASSERT(matches[2]->Result().Start() == 10);
		TEST_ASSERT(matches[2]->Result().Length() == 2);
		TEST_ASSERT(matches[2]->Result().Value() == L"56");

		matches.Clear();
		regex.Split(L"12abc34def56", false, matches);
		TEST_ASSERT(matches.Count() == 2);
		TEST_ASSERT(matches[0]->Success() == false);
		TEST_ASSERT(matches[0]->Result().Start() == 2);
		TEST_ASSERT(matches[0]->Result().Length() == 3);
		TEST_ASSERT(matches[0]->Result().Value() == L"abc");
		TEST_ASSERT(matches[1]->Success() == false);
		TEST_ASSERT(matches[1]->Result().Start() == 7);
		TEST_ASSERT(matches[1]->Result().Length() == 3);
		TEST_ASSERT(matches[1]->Result().Value() == L"def");

		matches.Clear();
		regex.Split(L"12abc34def56", true, matches);
		TEST_ASSERT(matches.Count() == 4);
		TEST_ASSERT(matches[0]->Success() == false);
		TEST_ASSERT(matches[0]->Result().Start() == 0);
		TEST_ASSERT(matches[0]->Result().Length() == 0);
		TEST_ASSERT(matches[0]->Result().Value() == L"");
		TEST_ASSERT(matches[1]->Success() == false);
		TEST_ASSERT(matches[1]->Result().Start() == 2);
		TEST_ASSERT(matches[1]->Result().Length() == 3);
		TEST_ASSERT(matches[1]->Result().Value() == L"abc");
		TEST_ASSERT(matches[2]->Success() == false);
		TEST_ASSERT(matches[2]->Result().Start() == 7);
		TEST_ASSERT(matches[2]->Result().Length() == 3);
		TEST_ASSERT(matches[2]->Result().Value() == L"def");
		TEST_ASSERT(matches[3]->Success() == false);
		TEST_ASSERT(matches[3]->Result().Start() == 12);
		TEST_ASSERT(matches[3]->Result().Length() == 0);
		TEST_ASSERT(matches[3]->Result().Value() == L"");

		matches.Clear();
		regex.Cut(L"12abc34def56", false, matches);
		TEST_ASSERT(matches.Count() == 5);
		TEST_ASSERT(matches[0]->Success() == true);
		TEST_ASSERT(matches[0]->Result().Start() == 0);
		TEST_ASSERT(matches[0]->Result().Length() == 2);
		TEST_ASSERT(matches[0]->Result().Value() == L"12");
		TEST_ASSERT(matches[1]->Success() == false);
		TEST_ASSERT(matches[1]->Result().Start() == 2);
		TEST_ASSERT(matches[1]->Result().Length() == 3);
		TEST_ASSERT(matches[1]->Result().Value() == L"abc");
		TEST_ASSERT(matches[2]->Success() == true);
		TEST_ASSERT(matches[2]->Result().Start() == 5);
		TEST_ASSERT(matches[2]->Result().Length() == 2);
		TEST_ASSERT(matches[2]->Result().Value() == L"34");
		TEST_ASSERT(matches[3]->Success() == false);
		TEST_ASSERT(matches[3]->Result().Start() == 7);
		TEST_ASSERT(matches[3]->Result().Length() == 3);
		TEST_ASSERT(matches[3]->Result().Value() == L"def");
		TEST_ASSERT(matches[4]->Success() == true);
		TEST_ASSERT(matches[4]->Result().Start() == 10);
		TEST_ASSERT(matches[4]->Result().Length() == 2);
		TEST_ASSERT(matches[4]->Result().Value() == L"56");

		matches.Clear();
		regex.Cut(L"12abc34def56", true, matches);
		TEST_ASSERT(matches.Count() == 7);
		TEST_ASSERT(matches[0]->Success() == false);
		TEST_ASSERT(matches[0]->Result().Start() == 0);
		TEST_ASSERT(matches[0]->Result().Length() == 0);
		TEST_ASSERT(matches[0]->Result().Value() == L"");
		TEST_ASSERT(matches[1]->Success() == true);
		TEST_ASSERT(matches[1]->Result().Start() == 0);
		TEST_ASSERT(matches[1]->Result().Length() == 2);
		TEST_ASSERT(matches[1]->Result().Value() == L"12");
		TEST_ASSERT(matches[2]->Success() == false);
		TEST_ASSERT(matches[2]->Result().Start() == 2);
		TEST_ASSERT(matches[2]->Result().Length() == 3);
		TEST_ASSERT(matches[2]->Result().Value() == L"abc");
		TEST_ASSERT(matches[3]->Success() == true);
		TEST_ASSERT(matches[3]->Result().Start() == 5);
		TEST_ASSERT(matches[3]->Result().Length() == 2);
		TEST_ASSERT(matches[3]->Result().Value() == L"34");
		TEST_ASSERT(matches[4]->Success() == false);
		TEST_ASSERT(matches[4]->Result().Start() == 7);
		TEST_ASSERT(matches[4]->Result().Length() == 3);
		TEST_ASSERT(matches[4]->Result().Value() == L"def");
		TEST_ASSERT(matches[5]->Success() == true);
		TEST_ASSERT(matches[5]->Result().Start() == 10);
		TEST_ASSERT(matches[5]->Result().Length() == 2);
		TEST_ASSERT(matches[5]->Result().Value() == L"56");
		TEST_ASSERT(matches[6]->Success() == false);
		TEST_ASSERT(matches[6]->Result().Start() == 12);
		TEST_ASSERT(matches[6]->Result().Length() == 0);
		TEST_ASSERT(matches[6]->Result().Value() == L"");

		matches.Clear();
		regex.Cut(L"XY12abc34def56ZW", true, matches);
		TEST_ASSERT(matches.Count() == 7);
		TEST_ASSERT(matches[0]->Success() == false);
		TEST_ASSERT(matches[0]->Result().Start() == 0);
		TEST_ASSERT(matches[0]->Result().Length() == 2);
		TEST_ASSERT(matches[0]->Result().Value() == L"XY");
		TEST_ASSERT(matches[1]->Success() == true);
		TEST_ASSERT(matches[1]->Result().Start() == 2);
		TEST_ASSERT(matches[1]->Result().Length() == 2);
		TEST_ASSERT(matches[1]->Result().Value() == L"12");
		TEST_ASSERT(matches[2]->Success() == false);
		TEST_ASSERT(matches[2]->Result().Start() == 4);
		TEST_ASSERT(matches[2]->Result().Length() == 3);
		TEST_ASSERT(matches[2]->Result().Value() == L"abc");
		TEST_ASSERT(matches[3]->Success() == true);
		TEST_ASSERT(matches[3]->Result().Start() == 7);
		TEST_ASSERT(matches[3]->Result().Length() == 2);
		TEST_ASSERT(matches[3]->Result().Value() == L"34");
		TEST_ASSERT(matches[4]->Success() == false);
		TEST_ASSERT(matches[4]->Result().Start() == 9);
		TEST_ASSERT(matches[4]->Result().Length() == 3);
		TEST_ASSERT(matches[4]->Result().Value() == L"def");
		TEST_ASSERT(matches[5]->Success() == true);
		TEST_ASSERT(matches[5]->Result().Start() == 12);
		TEST_ASSERT(matches[5]->Result().Length() == 2);
		TEST_ASSERT(matches[5]->Result().Value() == L"56");
		TEST_ASSERT(matches[6]->Success() == false);
		TEST_ASSERT(matches[6]->Result().Start() == 14);
		TEST_ASSERT(matches[6]->Result().Length() == 2);
		TEST_ASSERT(matches[6]->Result().Value() == L"ZW");
	};

	TEST_CASE(L"Test matching position")
	{
		TestRegexMatchPosition(true);
		TestRegexMatchPosition(false);
	});

	TEST_CASE(L"Test capturing")
	{
		{
			Regex regex(L"^(<a>/w+?)(<b>/w+?)((<$a>)(<$b>))+(<$a>)/w{6}$", true);
			TEST_ASSERT(regex.IsPureMatch() == false);
			TEST_ASSERT(regex.IsPureTest() == false);

			RegexMatch::Ref match = regex.Match(L"vczhgeniusvczhgeniusvczhgeniusvczhgenius");
			TEST_ASSERT(match);
			TEST_ASSERT(match->Success() == true);
			TEST_ASSERT(match->Result().Start() == 0);
			TEST_ASSERT(match->Result().Length() == 40);
			TEST_ASSERT(match->Result().Value() == L"vczhgeniusvczhgeniusvczhgeniusvczhgenius");
			TEST_ASSERT(match->Groups().Keys().Count() == 2);
			TEST_ASSERT(match->Groups().Keys()[0] == L"a");
			TEST_ASSERT(match->Groups().Keys()[1] == L"b");
			TEST_ASSERT(match->Groups()[L"a"].Count() == 1);
			TEST_ASSERT(match->Groups()[L"a"].Get(0).Start() == 0);
			TEST_ASSERT(match->Groups()[L"a"].Get(0).Length() == 4);
			TEST_ASSERT(match->Groups()[L"a"].Get(0).Value() == L"vczh");
			TEST_ASSERT(match->Groups()[L"b"].Count() == 1);
			TEST_ASSERT(match->Groups()[L"b"].Get(0).Start() == 4);
			TEST_ASSERT(match->Groups()[L"b"].Get(0).Length() == 6);
			TEST_ASSERT(match->Groups()[L"b"].Get(0).Value() == L"genius");
		}
		{
			Regex regex(L"^(?/d+).(?/d+).(?/d+).(<$0>).(<$1>).(<$2>)$");
			RegexMatch::Ref match;

			match = regex.MatchHead(L"12.34.56.12.56.34");
			TEST_ASSERT(!match);

			match = regex.MatchHead(L"123.4.56.123.4.56");
			TEST_ASSERT(match);
			TEST_ASSERT(match->Success() == true);
			TEST_ASSERT(match->Result().Start() == 0);
			TEST_ASSERT(match->Result().Length() == 17);
			TEST_ASSERT(match->Result().Value() == L"123.4.56.123.4.56");
			TEST_ASSERT(match->Captures().Count() == 3);
			TEST_ASSERT(match->Captures().Get(0).Start() == 0);
			TEST_ASSERT(match->Captures().Get(0).Length() == 3);
			TEST_ASSERT(match->Captures().Get(0).Value() == L"123");
			TEST_ASSERT(match->Captures().Get(1).Start() == 4);
			TEST_ASSERT(match->Captures().Get(1).Length() == 1);
			TEST_ASSERT(match->Captures().Get(1).Value() == L"4");
			TEST_ASSERT(match->Captures().Get(2).Start() == 6);
			TEST_ASSERT(match->Captures().Get(2).Length() == 2);
			TEST_ASSERT(match->Captures().Get(2).Value() == L"56");
		}
	});

#ifdef NDEBUG
	auto FindRows = [](WString* lines, int count, const WString& pattern)
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
		WString pattern = L"(\\.*A\\.*B\\.*C|\\.*A\\.*C\\.*B|\\.*B\\.*A\\.*C|\\.*B\\.*C\\.*A|\\.*C\\.*A\\.*B|\\.*C\\.*B\\.*A)";
		WString lines[] =
		{
			L"XAYBZC",
			L"XAYCZB",
			L"XBYAZC",
			L"XBYCZA",
			L"XCYAZB",
			L"XCYBZA",
		};
		FindRows(lines, sizeof(lines) / sizeof(*lines), pattern);
		FindRows(lines, sizeof(lines) / sizeof(*lines), pattern);
	});
#endif
}