#include <VlppOS.h>
#include "../../Source/Regex/AST/RegexExpression.h"
#define protected public
#include "../../Source/Regex/RegexPure.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::regex_internal;

namespace TestPure_TestObjects
{
	Ptr<PureInterpretor> BuildPureInterpretor(const char32_t* code)
	{
		CharRange::List subsets;
		Dictionary<State*, State*> nfaStateMap;
		Group<State*, State*> dfaStateMap;

		RegexExpression::Ref regex = ParseRegexExpression(code);
		Expression::Ref expression = regex->Merge();
		expression->NormalizeCharSet(subsets);
		Automaton::Ref eNfa = expression->GenerateEpsilonNfa();
		Automaton::Ref nfa = EpsilonNfaToNfa(eNfa, PureEpsilonChecker, nfaStateMap);
		Automaton::Ref dfa = NfaToDfa(nfa, dfaStateMap);
		return new PureInterpretor(dfa, subsets);
	}

	void RunPureInterpretor(const char32_t* code, const wchar_t* input, vint start, vint length)
	{
		PureResult matchResult;
		auto interpretor = BuildPureInterpretor(code);
		TEST_CASE(u32tow(code) + WString(L" on ") + input)
		{
			for (vint i = 0; i < 10; i++)
			{
				auto second = BuildPureInterpretor(code);
				TEST_ASSERT(interpretor->stateCount == second->stateCount);
				TEST_ASSERT(interpretor->charSetCount == second->charSetCount);
				TEST_ASSERT(interpretor->startState == second->startState);
				TEST_ASSERT(memcmp(interpretor->charMap, second->charMap, sizeof(vint) * PureInterpretor::SupportedCharCount) == 0);
				TEST_ASSERT(memcmp(interpretor->transitions, second->transitions, sizeof(vint) * (interpretor->stateCount * interpretor->charSetCount)) == 0);
				TEST_ASSERT(memcmp(interpretor->finalState, second->finalState, sizeof(bool) * interpretor->stateCount) == 0);
			}

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
using namespace TestPure_TestObjects;

TEST_FILE
{

	TEST_CATEGORY(L"Pure interpretor")
	{
		RunPureInterpretor(U"/d", L"abcde12345abcde", 5, 1);
		RunPureInterpretor(U"/d", L"12345abcde", 0, 1);
		RunPureInterpretor(U"/d", L"vczh", -1, 0);

		RunPureInterpretor(U"(/+|-)?/d+", L"abcde12345abcde", 5, 5);
		RunPureInterpretor(U"(/+|-)?/d+", L"abcde+12345abcde", 5, 6);
		RunPureInterpretor(U"(/+|-)?/d+", L"abcde-12345abcde", 5, 6);
		RunPureInterpretor(U"(/+|-)?/d+", L"12345abcde", 0, 5);
		RunPureInterpretor(U"(/+|-)?/d+", L"+12345abcde", 0, 6);
		RunPureInterpretor(U"(/+|-)?/d+", L"-12345abcde", 0, 6);
		RunPureInterpretor(U"(/+|-)?/d+", L"-+vczh+-", -1, 0);

		RunPureInterpretor(U"(/+|-)?/d+(./d+)?", L"abcde12345abcde", 5, 5);
		RunPureInterpretor(U"(/+|-)?/d+(./d+)?", L"abcde+12345abcde", 5, 6);
		RunPureInterpretor(U"(/+|-)?/d+(./d+)?", L"abcde-12345abcde", 5, 6);
		RunPureInterpretor(U"(/+|-)?/d+(./d+)?", L"abcde12345.abcde", 5, 5);
		RunPureInterpretor(U"(/+|-)?/d+(./d+)?", L"abcde+12345.abcde", 5, 6);
		RunPureInterpretor(U"(/+|-)?/d+(./d+)?", L"abcde-12345.abcde", 5, 6);
		RunPureInterpretor(U"(/+|-)?/d+(./d+)?", L"abcde12345.54321abcde", 5, 11);
		RunPureInterpretor(U"(/+|-)?/d+(./d+)?", L"abcde+12345.54321abcde", 5, 12);
		RunPureInterpretor(U"(/+|-)?/d+(./d+)?", L"abcde-12345.54321abcde", 5, 12);
		RunPureInterpretor(U"(/+|-)?/d+(./d+)?", L"12345", 0, 5);
		RunPureInterpretor(U"(/+|-)?/d+(./d+)?", L"+12345", 0, 6);
		RunPureInterpretor(U"(/+|-)?/d+(./d+)?", L"-12345", 0, 6);
		RunPureInterpretor(U"(/+|-)?/d+(./d+)?", L"12345.", 0, 5);
		RunPureInterpretor(U"(/+|-)?/d+(./d+)?", L"+12345.", 0, 6);
		RunPureInterpretor(U"(/+|-)?/d+(./d+)?", L"-12345.", 0, 6);
		RunPureInterpretor(U"(/+|-)?/d+(./d+)?", L"12345.54321", 0, 11);
		RunPureInterpretor(U"(/+|-)?/d+(./d+)?", L"+12345.54321", 0, 12);
		RunPureInterpretor(U"(/+|-)?/d+(./d+)?", L"-12345.54321", 0, 12);
		RunPureInterpretor(U"(/+|-)?/d+(./d+)?", L"-+vczh+-", -1, 0);

		RunPureInterpretor(U"\"([^\\\\\"]|\\\\\\.)*\"", L"vczh\"is\"genius", 4, 4);
		RunPureInterpretor(U"\"([^\\\\\"]|\\\\\\.)*\"", L"vczh\"i\\r\\ns\"genius", 4, 8);
		RunPureInterpretor(U"\"([^\\\\\"]|\\\\\\.)*\"", L"vczh is genius", -1, 0);

		RunPureInterpretor(U"///*([^*]|/*+[^*//])*/*+//", L"vczh/*is*/genius", 4, 6);
		RunPureInterpretor(U"///*([^*]|/*+[^*//])*/*+//", L"vczh/***is***/genius", 4, 10);
		RunPureInterpretor(U"///*([^*]|/*+[^*//])*/*+//", L"vczh is genius", -1, 0);
	});

	TEST_CATEGORY(L"Unicode")
	{
		auto interpretor = BuildPureInterpretor(U"[𣂕𣴑𣱳𦁚]+");

		TEST_CASE(L"char8_t")
		{
			auto input = u8"𩰪㦲𦰗𠀼 𣂕𣴑𣱳𦁚 Vczh is genius!@我是天才";
			PureResult result;
			interpretor->Match(input, input, result);
			TEST_ASSERT(result.start == 16);
			TEST_ASSERT(result.length == 16);
		});

		TEST_CASE(L"char16_t")
		{
			auto input = u"𩰪㦲𦰗𠀼 𣂕𣴑𣱳𦁚 Vczh is genius!@我是天才";
			PureResult result;
			interpretor->Match(input, input, result);
			TEST_ASSERT(result.start == 8);
			TEST_ASSERT(result.length == 8);
		});

		TEST_CASE(L"char32_t")
		{
			auto input = U"𩰪㦲𦰗𠀼 𣂕𣴑𣱳𦁚 Vczh is genius!@我是天才";
			PureResult result;
			interpretor->Match(input, input, result);
			TEST_ASSERT(result.start == 5);
			TEST_ASSERT(result.length == 4);
		});
	});
}