#include "../../Source/Regex/AST/RegexExpression.h"
#include "../../Source/Regex/RegexRich.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::regex_internal;

TEST_FILE
{
	auto BuildRichInterpretor = [](const wchar_t* code)
	{
		CharRange::List subsets;
		Dictionary<State*, State*> nfaStateMap;
		Group<State*, State*> dfaStateMap;

		RegexExpression::Ref regex = ParseRegexExpression(code);
		Expression::Ref expression = regex->Merge();
		expression->NormalizeCharSet(subsets);
		Automaton::Ref eNfa = expression->GenerateEpsilonNfa();
		Automaton::Ref nfa = EpsilonNfaToNfa(eNfa, RichEpsilonChecker, nfaStateMap);
		Automaton::Ref dfa = NfaToDfa(nfa, dfaStateMap);

		return new RichInterpretor(dfa);
	};

	auto RunRichInterpretor = [&](const wchar_t* code, const wchar_t* input, vint start, vint length)
	{
		TEST_CASE(code + WString(L" on ") + input)
		{
			RichResult matchResult;
			Ptr<RichInterpretor> interpretor = BuildRichInterpretor(code);
			bool expectedSuccessful = start != -1;
			TEST_ASSERT(interpretor->Match(input, input, matchResult) == expectedSuccessful);
			if (expectedSuccessful)
			{
				TEST_ASSERT(start == matchResult.start);
				TEST_ASSERT(length == matchResult.length);
			}
		});
	};

	TEST_CATEGORY(L"Rich interpretor: simple")
	{
		RunRichInterpretor(L"/d", L"abcde12345abcde", 5, 1);
		RunRichInterpretor(L"/d", L"12345abcde", 0, 1);
		RunRichInterpretor(L"/d", L"vczh", -1, 0);

		RunRichInterpretor(L"(/+|-)?/d+", L"abcde12345abcde", 5, 5);
		RunRichInterpretor(L"(/+|-)?/d+", L"abcde+12345abcde", 5, 6);
		RunRichInterpretor(L"(/+|-)?/d+", L"abcde-12345abcde", 5, 6);
		RunRichInterpretor(L"(/+|-)?/d+", L"12345abcde", 0, 5);
		RunRichInterpretor(L"(/+|-)?/d+", L"+12345abcde", 0, 6);
		RunRichInterpretor(L"(/+|-)?/d+", L"-12345abcde", 0, 6);
		RunRichInterpretor(L"(/+|-)?/d+", L"-+vczh+-", -1, 0);

		RunRichInterpretor(L"(/+|-)?/d+(./d+)?", L"abcde12345abcde", 5, 5);
		RunRichInterpretor(L"(/+|-)?/d+(./d+)?", L"abcde+12345abcde", 5, 6);
		RunRichInterpretor(L"(/+|-)?/d+(./d+)?", L"abcde-12345abcde", 5, 6);
		RunRichInterpretor(L"(/+|-)?/d+(./d+)?", L"abcde12345.abcde", 5, 5);
		RunRichInterpretor(L"(/+|-)?/d+(./d+)?", L"abcde+12345.abcde", 5, 6);
		RunRichInterpretor(L"(/+|-)?/d+(./d+)?", L"abcde-12345.abcde", 5, 6);
		RunRichInterpretor(L"(/+|-)?/d+(./d+)?", L"abcde12345.54321abcde", 5, 11);
		RunRichInterpretor(L"(/+|-)?/d+(./d+)?", L"abcde+12345.54321abcde", 5, 12);
		RunRichInterpretor(L"(/+|-)?/d+(./d+)?", L"abcde-12345.54321abcde", 5, 12);
		RunRichInterpretor(L"(/+|-)?/d+(./d+)?", L"12345", 0, 5);
		RunRichInterpretor(L"(/+|-)?/d+(./d+)?", L"+12345", 0, 6);
		RunRichInterpretor(L"(/+|-)?/d+(./d+)?", L"-12345", 0, 6);
		RunRichInterpretor(L"(/+|-)?/d+(./d+)?", L"12345.", 0, 5);
		RunRichInterpretor(L"(/+|-)?/d+(./d+)?", L"+12345.", 0, 6);
		RunRichInterpretor(L"(/+|-)?/d+(./d+)?", L"-12345.", 0, 6);
		RunRichInterpretor(L"(/+|-)?/d+(./d+)?", L"12345.54321", 0, 11);
		RunRichInterpretor(L"(/+|-)?/d+(./d+)?", L"+12345.54321", 0, 12);
		RunRichInterpretor(L"(/+|-)?/d+(./d+)?", L"-12345.54321", 0, 12);
		RunRichInterpretor(L"(/+|-)?/d+(./d+)?", L"-+vczh+-", -1, 0);

		RunRichInterpretor(L"\"([^\\\\\"]|\\\\\\.)*\"", L"vczh\"is\"genius", 4, 4);
		RunRichInterpretor(L"\"([^\\\\\"]|\\\\\\.)*\"", L"vczh\"i\\r\\ns\"genius", 4, 8);
		RunRichInterpretor(L"\"([^\\\\\"]|\\\\\\.)*\"", L"vczh is genius", -1, 0);

		RunRichInterpretor(L"///*([^*]|/*+[^*//])*/*+//", L"vczh/*is*/genius", 4, 6);
		RunRichInterpretor(L"///*([^*]|/*+[^*//])*/*+//", L"vczh/***is***/genius", 4, 10);
		RunRichInterpretor(L"///*([^*]|/*+[^*//])*/*+//", L"vczh is genius", -1, 0);
	});

	TEST_CATEGORY(L"Rich interpretor: ^ / $")
	{
		RunRichInterpretor(L"/d+", L"abc1234abc", 3, 4);
		RunRichInterpretor(L"/d+", L"1234abc", 0, 4);
		RunRichInterpretor(L"/d+", L"abc1234", 3, 4);
		RunRichInterpretor(L"/d+", L"1234", 0, 4);

		RunRichInterpretor(L"^/d+", L"abc1234abc", -1, 0);
		RunRichInterpretor(L"^/d+", L"1234abc", 0, 4);
		RunRichInterpretor(L"^/d+", L"abc1234", -1, 0);
		RunRichInterpretor(L"^/d+", L"1234", 0, 4);

		RunRichInterpretor(L"/d+$", L"abc1234abc", -1, 0);
		RunRichInterpretor(L"/d+$", L"1234abc", -1, 0);
		RunRichInterpretor(L"/d+$", L"abc1234", 3, 4);
		RunRichInterpretor(L"/d+$", L"1234", 0, 4);

		RunRichInterpretor(L"^/d+$", L"abc1234abc", -1, 0);
		RunRichInterpretor(L"^/d+$", L"1234abc", -1, 0);
		RunRichInterpretor(L"^/d+$", L"abc1234", -1, 0);
		RunRichInterpretor(L"^/d+$", L"1234", 0, 4);
	});

	TEST_CATEGORY(L"Rich interpretor: looping")
	{
		RunRichInterpretor(L"/d+", L"abcde12345abcde", 5, 5);
		RunRichInterpretor(L"/d+?", L"abcde12345abcde", 5, 1);
		RunRichInterpretor(L"/d+a", L"abcde12345abcde", 5, 6);
		RunRichInterpretor(L"/d+?a", L"abcde12345abcde", 5, 6);
	});

	TEST_CATEGORY(L"Rich interpretor: capturing")
	{
		TEST_CASE(L"(<number>/d+) on abcde123456abcde")
		{
			const wchar_t* code = L"(<number>/d+)";
			const wchar_t* input = L"abcde123456abcde";
			RichResult result;
			Ptr<RichInterpretor> regex = BuildRichInterpretor(code);
			vint index = regex->CaptureNames().IndexOf(L"number");
			TEST_ASSERT(index == 0);

			TEST_ASSERT(regex->Match(input, input, result) == true);
			TEST_ASSERT(result.start == 5);
			TEST_ASSERT(result.length == 6);
			TEST_ASSERT(result.captures.Count() == 1);
			TEST_ASSERT(result.captures[0].capture == 0);
			TEST_ASSERT(result.captures[0].start == 5);
			TEST_ASSERT(result.captures[0].length == 6);
		});

		TEST_CASE(L"(<#sec>(<sec>/d+))((<&sec>).){3}(<&sec>) on 196.128.0.1")
		{
			const wchar_t* code = L"(<#sec>(<sec>/d+))((<&sec>).){3}(<&sec>)";
			const wchar_t* input = L"196.128.0.1";
			RichResult result;
			Ptr<RichInterpretor> regex = BuildRichInterpretor(code);
			vint index = regex->CaptureNames().IndexOf(L"sec");
			TEST_ASSERT(index == 0);

			TEST_ASSERT(regex->Match(input, input, result) == true);
			TEST_ASSERT(result.start == 0);
			TEST_ASSERT(result.length == 11);
			TEST_ASSERT(result.captures.Count() == 4);
			TEST_ASSERT(result.captures[0].capture == 0);
			TEST_ASSERT(result.captures[0].start == 0);
			TEST_ASSERT(result.captures[0].length == 3);
			TEST_ASSERT(result.captures[1].capture == 0);
			TEST_ASSERT(result.captures[1].start == 4);
			TEST_ASSERT(result.captures[1].length == 3);
			TEST_ASSERT(result.captures[2].capture == 0);
			TEST_ASSERT(result.captures[2].start == 8);
			TEST_ASSERT(result.captures[2].length == 1);
			TEST_ASSERT(result.captures[3].capture == 0);
			TEST_ASSERT(result.captures[3].start == 10);
			TEST_ASSERT(result.captures[3].length == 1);
		});

		TEST_CASE(L"(<sec>/d+?)(<$sec>)+ on 98765123123123123123123")
		{
			const wchar_t* code = L"(<sec>/d+?)(<$sec>)+";
			const wchar_t* input = L"98765123123123123123123";
			RichResult result;
			Ptr<RichInterpretor> regex = BuildRichInterpretor(code);
			vint index = regex->CaptureNames().IndexOf(L"sec");
			TEST_ASSERT(index == 0);

			TEST_ASSERT(regex->Match(input, input, result) == true);
			TEST_ASSERT(result.start == 5);
			TEST_ASSERT(result.length == 18);
			TEST_ASSERT(result.captures.Count() == 1);
			TEST_ASSERT(result.captures[0].capture == 0);
			TEST_ASSERT(result.captures[0].start == 5);
			TEST_ASSERT(result.captures[0].length == 3);
		});

		TEST_CASE(L"(<sec>/d+)(<$sec>)+ on 98765123123123123123123")
		{
			const wchar_t* code = L"(<sec>/d+)(<$sec>)+";
			const wchar_t* input = L"98765123123123123123123";
			RichResult result;
			Ptr<RichInterpretor> regex = BuildRichInterpretor(code);
			vint index = regex->CaptureNames().IndexOf(L"sec");
			TEST_ASSERT(index == 0);

			TEST_ASSERT(regex->Match(input, input, result) == true);
			TEST_ASSERT(result.start == 5);
			TEST_ASSERT(result.length == 18);
			TEST_ASSERT(result.captures.Count() == 1);
			TEST_ASSERT(result.captures[0].capture == 0);
			TEST_ASSERT(result.captures[0].start == 5);
			TEST_ASSERT(result.captures[0].length == 9);
		});
	});

	TEST_CATEGORY(L"Rich interpretor: prematching")
	{
		TEST_CASE(L"win(=2000) on win98win2000winxp")
		{
			const wchar_t* code = L"win(=2000)";
			const wchar_t* input = L"win98win2000winxp";
			RichResult result;
			Ptr<RichInterpretor> regex = BuildRichInterpretor(code);

			TEST_ASSERT(regex->Match(input, input, result) == true);
			TEST_ASSERT(result.start == 5);
			TEST_ASSERT(result.length == 3);
			TEST_ASSERT(result.captures.Count() == 0);
		});

		TEST_CASE(L"win(!98) on win98win2000winxp")
		{
			const wchar_t* code = L"win(!98)";
			const wchar_t* input = L"win98win2000winxp";
			RichResult result;
			Ptr<RichInterpretor> regex = BuildRichInterpretor(code);

			TEST_ASSERT(regex->Match(input, input, result) == true);
			TEST_ASSERT(result.start == 5);
			TEST_ASSERT(result.length == 3);
			TEST_ASSERT(result.captures.Count() == 0);
		});
	});

	TEST_CATEGORY(L"Rich interpretor: others")
	{
		TEST_CASE(L"^(<a>/w+?)(<b>/w+?)((<$a>)(<$b>))+(<$a>)/w{6}$ on vczhgeniusvczhgeniusvczhgeniusvczhgenius")
		{
			const wchar_t* code = L"^(<a>/w+?)(<b>/w+?)((<$a>)(<$b>))+(<$a>)/w{6}$";
			const wchar_t* input = L"vczhgeniusvczhgeniusvczhgeniusvczhgenius";
			RichResult result;
			Ptr<RichInterpretor> regex = BuildRichInterpretor(code);
			TEST_ASSERT(regex->CaptureNames().IndexOf(L"a") == 0);
			TEST_ASSERT(regex->CaptureNames().IndexOf(L"b") == 1);

			TEST_ASSERT(regex->Match(input, input, result) == true);
			TEST_ASSERT(result.start == 0);
			TEST_ASSERT(result.length == 40);
			TEST_ASSERT(result.captures.Count() == 2);
			TEST_ASSERT(result.captures[0].capture == 0);
			TEST_ASSERT(result.captures[0].start == 0);
			TEST_ASSERT(result.captures[0].length == 4);
			TEST_ASSERT(result.captures[1].capture == 1);
			TEST_ASSERT(result.captures[1].start == 4);
			TEST_ASSERT(result.captures[1].length == 6);
		});

		TEST_CASE(L"^/d+./d*?(<sec>/d+?)(<$sec>)+$ on 1428.57142857142857142857")
		{
			const wchar_t* code = L"^/d+./d*?(<sec>/d+?)(<$sec>)+$";
			const wchar_t* input = L"1428.57142857142857142857";
			RichResult result;
			Ptr<RichInterpretor> regex = BuildRichInterpretor(code);
			TEST_ASSERT(regex->CaptureNames().IndexOf(L"sec") == 0);

			TEST_ASSERT(regex->Match(input, input, result) == true);
			TEST_ASSERT(result.start == 0);
			TEST_ASSERT(result.length == 25);
			TEST_ASSERT(result.captures.Count() == 1);
			TEST_ASSERT(result.captures[0].capture == 0);
			TEST_ASSERT(result.captures[0].start == 7);
			TEST_ASSERT(result.captures[0].length == 6);
		});

		TEST_CASE(L"^/d+./d*?(?/d+?)(<$0>)+$ on 1428.57142857142857142857")
		{
			const wchar_t* code = L"^/d+./d*?(?/d+?)(<$0>)+$";
			const wchar_t* input = L"1428.57142857142857142857";
			RichResult result;
			Ptr<RichInterpretor> regex = BuildRichInterpretor(code);

			TEST_ASSERT(regex->Match(input, input, result) == true);
			TEST_ASSERT(result.start == 0);
			TEST_ASSERT(result.length == 25);
			TEST_ASSERT(result.captures.Count() == 1);
			TEST_ASSERT(result.captures[0].capture == -1);
			TEST_ASSERT(result.captures[0].start == 7);
			TEST_ASSERT(result.captures[0].length == 6);
		});
	});
}