﻿#include "../../Source/Regex/AST/RegexExpression.h"
#include "../../Source/Regex/RegexRich.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::regex_internal;

namespace TestRich_TestObjects
{
	Ptr<RichInterpretor> BuildRichInterpretor(const char32_t* code)
	{
		CharRange::List subsets;
		Dictionary<State*, State*> nfaStateMap;
		Group<State*, State*> dfaStateMap;

		auto regex = ParseRegexExpression(code);
		auto expression = regex->Merge();
		expression->NormalizeCharSet(subsets);
		auto eNfa = expression->GenerateEpsilonNfa();
		auto nfa = EpsilonNfaToNfa(eNfa, RichEpsilonChecker, nfaStateMap);
		auto dfa = NfaToDfa(nfa, dfaStateMap);

		return Ptr(new RichInterpretor(dfa));
	}

	void RunRichInterpretor(const char32_t* code, const wchar_t* input, vint start, vint length)
	{
		TEST_CASE(u32tow(code) + WString(L" on ") + input)
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
	}
}
using namespace TestRich_TestObjects;

TEST_FILE
{

	TEST_CATEGORY(L"Rich interpretor: simple")
	{
		RunRichInterpretor(U"/d", L"abcde12345abcde", 5, 1);
		RunRichInterpretor(U"/d", L"12345abcde", 0, 1);
		RunRichInterpretor(U"/d", L"vczh", -1, 0);

		RunRichInterpretor(U"(/+|-)?/d+", L"abcde12345abcde", 5, 5);
		RunRichInterpretor(U"(/+|-)?/d+", L"abcde+12345abcde", 5, 6);
		RunRichInterpretor(U"(/+|-)?/d+", L"abcde-12345abcde", 5, 6);
		RunRichInterpretor(U"(/+|-)?/d+", L"12345abcde", 0, 5);
		RunRichInterpretor(U"(/+|-)?/d+", L"+12345abcde", 0, 6);
		RunRichInterpretor(U"(/+|-)?/d+", L"-12345abcde", 0, 6);
		RunRichInterpretor(U"(/+|-)?/d+", L"-+vczh+-", -1, 0);

		RunRichInterpretor(U"(/+|-)?/d+(./d+)?", L"abcde12345abcde", 5, 5);
		RunRichInterpretor(U"(/+|-)?/d+(./d+)?", L"abcde+12345abcde", 5, 6);
		RunRichInterpretor(U"(/+|-)?/d+(./d+)?", L"abcde-12345abcde", 5, 6);
		RunRichInterpretor(U"(/+|-)?/d+(./d+)?", L"abcde12345.abcde", 5, 5);
		RunRichInterpretor(U"(/+|-)?/d+(./d+)?", L"abcde+12345.abcde", 5, 6);
		RunRichInterpretor(U"(/+|-)?/d+(./d+)?", L"abcde-12345.abcde", 5, 6);
		RunRichInterpretor(U"(/+|-)?/d+(./d+)?", L"abcde12345.54321abcde", 5, 11);
		RunRichInterpretor(U"(/+|-)?/d+(./d+)?", L"abcde+12345.54321abcde", 5, 12);
		RunRichInterpretor(U"(/+|-)?/d+(./d+)?", L"abcde-12345.54321abcde", 5, 12);
		RunRichInterpretor(U"(/+|-)?/d+(./d+)?", L"12345", 0, 5);
		RunRichInterpretor(U"(/+|-)?/d+(./d+)?", L"+12345", 0, 6);
		RunRichInterpretor(U"(/+|-)?/d+(./d+)?", L"-12345", 0, 6);
		RunRichInterpretor(U"(/+|-)?/d+(./d+)?", L"12345.", 0, 5);
		RunRichInterpretor(U"(/+|-)?/d+(./d+)?", L"+12345.", 0, 6);
		RunRichInterpretor(U"(/+|-)?/d+(./d+)?", L"-12345.", 0, 6);
		RunRichInterpretor(U"(/+|-)?/d+(./d+)?", L"12345.54321", 0, 11);
		RunRichInterpretor(U"(/+|-)?/d+(./d+)?", L"+12345.54321", 0, 12);
		RunRichInterpretor(U"(/+|-)?/d+(./d+)?", L"-12345.54321", 0, 12);
		RunRichInterpretor(U"(/+|-)?/d+(./d+)?", L"-+vczh+-", -1, 0);

		RunRichInterpretor(U"\"([^\\\\\"]|\\\\\\.)*\"", L"vczh\"is\"genius", 4, 4);
		RunRichInterpretor(U"\"([^\\\\\"]|\\\\\\.)*\"", L"vczh\"i\\r\\ns\"genius", 4, 8);
		RunRichInterpretor(U"\"([^\\\\\"]|\\\\\\.)*\"", L"vczh is genius", -1, 0);

		RunRichInterpretor(U"///*([^*]|/*+[^*//])*/*+//", L"vczh/*is*/genius", 4, 6);
		RunRichInterpretor(U"///*([^*]|/*+[^*//])*/*+//", L"vczh/***is***/genius", 4, 10);
		RunRichInterpretor(U"///*([^*]|/*+[^*//])*/*+//", L"vczh is genius", -1, 0);
	});

	TEST_CATEGORY(L"Rich interpretor: ^ / $")
	{
		RunRichInterpretor(U"/d+", L"abc1234abc", 3, 4);
		RunRichInterpretor(U"/d+", L"1234abc", 0, 4);
		RunRichInterpretor(U"/d+", L"abc1234", 3, 4);
		RunRichInterpretor(U"/d+", L"1234", 0, 4);

		RunRichInterpretor(U"^/d+", L"abc1234abc", -1, 0);
		RunRichInterpretor(U"^/d+", L"1234abc", 0, 4);
		RunRichInterpretor(U"^/d+", L"abc1234", -1, 0);
		RunRichInterpretor(U"^/d+", L"1234", 0, 4);

		RunRichInterpretor(U"/d+$", L"abc1234abc", -1, 0);
		RunRichInterpretor(U"/d+$", L"1234abc", -1, 0);
		RunRichInterpretor(U"/d+$", L"abc1234", 3, 4);
		RunRichInterpretor(U"/d+$", L"1234", 0, 4);

		RunRichInterpretor(U"^/d+$", L"abc1234abc", -1, 0);
		RunRichInterpretor(U"^/d+$", L"1234abc", -1, 0);
		RunRichInterpretor(U"^/d+$", L"abc1234", -1, 0);
		RunRichInterpretor(U"^/d+$", L"1234", 0, 4);
	});

	TEST_CATEGORY(L"Rich interpretor: looping")
	{
		RunRichInterpretor(U"/d+", L"abcde12345abcde", 5, 5);
		RunRichInterpretor(U"/d+?", L"abcde12345abcde", 5, 1);
		RunRichInterpretor(U"/d+a", L"abcde12345abcde", 5, 6);
		RunRichInterpretor(U"/d+?a", L"abcde12345abcde", 5, 6);
	});

	TEST_CATEGORY(L"Rich interpretor: capturing")
	{
		TEST_CASE(L"(<number>/d+) on abcde123456abcde")
		{
			const char32_t* code = U"(<number>/d+)";
			const wchar_t* input = L"abcde123456abcde";
			RichResult result;
			auto regex = BuildRichInterpretor(code);
			vint index = regex->CaptureNames().IndexOf(U"number");
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
			const char32_t* code = U"(<#sec>(<sec>/d+))((<&sec>).){3}(<&sec>)";
			const wchar_t* input = L"196.128.0.1";
			RichResult result;
			auto regex = BuildRichInterpretor(code);
			vint index = regex->CaptureNames().IndexOf(U"sec");
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
			const char32_t* code = U"(<sec>/d+?)(<$sec>)+";
			const wchar_t* input = L"98765123123123123123123";
			RichResult result;
			auto regex = BuildRichInterpretor(code);
			vint index = regex->CaptureNames().IndexOf(U"sec");
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
			const char32_t* code = U"(<sec>/d+)(<$sec>)+";
			const wchar_t* input = L"98765123123123123123123";
			RichResult result;
			auto regex = BuildRichInterpretor(code);
			vint index = regex->CaptureNames().IndexOf(U"sec");
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
			const char32_t* code = U"win(=2000)";
			const wchar_t* input = L"win98win2000winxp";
			RichResult result;
			auto regex = BuildRichInterpretor(code);

			TEST_ASSERT(regex->Match(input, input, result) == true);
			TEST_ASSERT(result.start == 5);
			TEST_ASSERT(result.length == 3);
			TEST_ASSERT(result.captures.Count() == 0);
		});

		TEST_CASE(L"win(!98) on win98win2000winxp")
		{
			const char32_t* code = U"win(!98)";
			const wchar_t* input = L"win98win2000winxp";
			RichResult result;
			auto regex = BuildRichInterpretor(code);

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
			const char32_t* code = U"^(<a>/w+?)(<b>/w+?)((<$a>)(<$b>))+(<$a>)/w{6}$";
			const wchar_t* input = L"vczhgeniusvczhgeniusvczhgeniusvczhgenius";
			RichResult result;
			auto regex = BuildRichInterpretor(code);
			TEST_ASSERT(regex->CaptureNames().IndexOf(U"a") == 0);
			TEST_ASSERT(regex->CaptureNames().IndexOf(U"b") == 1);

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
			const char32_t* code = U"^/d+./d*?(<sec>/d+?)(<$sec>)+$";
			const wchar_t* input = L"1428.57142857142857142857";
			RichResult result;
			auto regex = BuildRichInterpretor(code);
			TEST_ASSERT(regex->CaptureNames().IndexOf(U"sec") == 0);

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
			const char32_t* code = U"^/d+./d*?(?/d+?)(<$0>)+$";
			const wchar_t* input = L"1428.57142857142857142857";
			RichResult result;
			auto regex = BuildRichInterpretor(code);

			TEST_ASSERT(regex->Match(input, input, result) == true);
			TEST_ASSERT(result.start == 0);
			TEST_ASSERT(result.length == 25);
			TEST_ASSERT(result.captures.Count() == 1);
			TEST_ASSERT(result.captures[0].capture == -1);
			TEST_ASSERT(result.captures[0].start == 7);
			TEST_ASSERT(result.captures[0].length == 6);
		});
	});

	TEST_CATEGORY(L"Unicode")
	{
		auto interpretor = BuildRichInterpretor(U"/./.(?[𣂕𣴑𣱳𦁚]+)/./.");

		TEST_CASE(L"char8_t")
		{
			auto input = u8"𩰪㦲𦰗𠀼 𣂕𣴑𣱳𦁚 Vczh is genius!@我是天才";
			RichResult result;
			interpretor->Match(input, input, result);
			TEST_ASSERT(result.start == 11);
			TEST_ASSERT(result.length == 23);
			TEST_ASSERT(result.captures.Count() == 1);
			TEST_ASSERT(result.captures[0].capture == -1);
			TEST_ASSERT(result.captures[0].start == 16);
			TEST_ASSERT(result.captures[0].length == 16);
		});

		TEST_CASE(L"char16_t")
		{
			auto input = u"𩰪㦲𦰗𠀼 𣂕𣴑𣱳𦁚 Vczh is genius!@我是天才";
			RichResult result;
			interpretor->Match(input, input, result);
			TEST_ASSERT(result.start == 5);
			TEST_ASSERT(result.length == 13);
			TEST_ASSERT(result.captures.Count() == 1);
			TEST_ASSERT(result.captures[0].capture == -1);
			TEST_ASSERT(result.captures[0].start == 8);
			TEST_ASSERT(result.captures[0].length == 8);
		});

		TEST_CASE(L"char32_t")
		{
			auto input = U"𩰪㦲𦰗𠀼 𣂕𣴑𣱳𦁚 Vczh is genius!@我是天才";
			RichResult result;
			interpretor->Match(input, input, result);
			TEST_ASSERT(result.start == 3);
			TEST_ASSERT(result.length == 8);
			TEST_ASSERT(result.captures.Count() == 1);
			TEST_ASSERT(result.captures[0].capture == -1);
			TEST_ASSERT(result.captures[0].start == 5);
			TEST_ASSERT(result.captures[0].length == 4);
		});
	});
}