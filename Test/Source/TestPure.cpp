#include "../../Source/Regex/AST/RegexExpression.h"
#include "../../Source/Regex/RegexPure.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::regex_internal;

TEST_FILE
{
	auto RunPureInterpretor = [](const char32_t* code, const wchar_t* input, vint start, vint length)
	{
		TEST_CASE(u32tow(code) + WString(L" on ") + input)
		{
			CharRange::List subsets;
			Dictionary<State*, State*> nfaStateMap;
			Group<State*, State*> dfaStateMap;
			PureResult matchResult;

			RegexExpression::Ref regex = ParseRegexExpression(code);
			Expression::Ref expression = regex->Merge();
			expression->NormalizeCharSet(subsets);
			Automaton::Ref eNfa = expression->GenerateEpsilonNfa();
			Automaton::Ref nfa = EpsilonNfaToNfa(eNfa, PureEpsilonChecker, nfaStateMap);
			Automaton::Ref dfa = NfaToDfa(nfa, dfaStateMap);

			Ptr<PureInterpretor> interpretor = new PureInterpretor(dfa, subsets);
			bool expectedSuccessful = start != -1;
			TEST_ASSERT(interpretor->Match(input, input, matchResult) == expectedSuccessful);
			if (expectedSuccessful)
			{
				TEST_ASSERT(start == matchResult.start);
				TEST_ASSERT(length == matchResult.length);
			}
		});
	};

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
}