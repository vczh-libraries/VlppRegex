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
	void RunPureInterpretor(const wchar_t* code, const wchar_t* input, vint start, vint length)
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
	}

	TEST_CASE(TestPureInterpretor)
	{
		RunPureInterpretor(L"/d", L"abcde12345abcde", 5, 1);
		RunPureInterpretor(L"/d", L"12345abcde", 0, 1);
		RunPureInterpretor(L"/d", L"vczh", -1, 0);

		RunPureInterpretor(L"(/+|-)?/d+", L"abcde12345abcde", 5, 5);
		RunPureInterpretor(L"(/+|-)?/d+", L"abcde+12345abcde", 5, 6);
		RunPureInterpretor(L"(/+|-)?/d+", L"abcde-12345abcde", 5, 6);
		RunPureInterpretor(L"(/+|-)?/d+", L"12345abcde", 0, 5);
		RunPureInterpretor(L"(/+|-)?/d+", L"+12345abcde", 0, 6);
		RunPureInterpretor(L"(/+|-)?/d+", L"-12345abcde", 0, 6);
		RunPureInterpretor(L"(/+|-)?/d+", L"-+vczh+-", -1, 0);

		RunPureInterpretor(L"(/+|-)?/d+(./d+)?", L"abcde12345abcde", 5, 5);
		RunPureInterpretor(L"(/+|-)?/d+(./d+)?", L"abcde+12345abcde", 5, 6);
		RunPureInterpretor(L"(/+|-)?/d+(./d+)?", L"abcde-12345abcde", 5, 6);
		RunPureInterpretor(L"(/+|-)?/d+(./d+)?", L"abcde12345.abcde", 5, 5);
		RunPureInterpretor(L"(/+|-)?/d+(./d+)?", L"abcde+12345.abcde", 5, 6);
		RunPureInterpretor(L"(/+|-)?/d+(./d+)?", L"abcde-12345.abcde", 5, 6);
		RunPureInterpretor(L"(/+|-)?/d+(./d+)?", L"abcde12345.54321abcde", 5, 11);
		RunPureInterpretor(L"(/+|-)?/d+(./d+)?", L"abcde+12345.54321abcde", 5, 12);
		RunPureInterpretor(L"(/+|-)?/d+(./d+)?", L"abcde-12345.54321abcde", 5, 12);
		RunPureInterpretor(L"(/+|-)?/d+(./d+)?", L"12345", 0, 5);
		RunPureInterpretor(L"(/+|-)?/d+(./d+)?", L"+12345", 0, 6);
		RunPureInterpretor(L"(/+|-)?/d+(./d+)?", L"-12345", 0, 6);
		RunPureInterpretor(L"(/+|-)?/d+(./d+)?", L"12345.", 0, 5);
		RunPureInterpretor(L"(/+|-)?/d+(./d+)?", L"+12345.", 0, 6);
		RunPureInterpretor(L"(/+|-)?/d+(./d+)?", L"-12345.", 0, 6);
		RunPureInterpretor(L"(/+|-)?/d+(./d+)?", L"12345.54321", 0, 11);
		RunPureInterpretor(L"(/+|-)?/d+(./d+)?", L"+12345.54321", 0, 12);
		RunPureInterpretor(L"(/+|-)?/d+(./d+)?", L"-12345.54321", 0, 12);
		RunPureInterpretor(L"(/+|-)?/d+(./d+)?", L"-+vczh+-", -1, 0);

		RunPureInterpretor(L"\"([^\\\\\"]|\\\\\\.)*\"", L"vczh\"is\"genius", 4, 4);
		RunPureInterpretor(L"\"([^\\\\\"]|\\\\\\.)*\"", L"vczh\"i\\r\\ns\"genius", 4, 8);
		RunPureInterpretor(L"\"([^\\\\\"]|\\\\\\.)*\"", L"vczh is genius", -1, 0);

		RunPureInterpretor(L"///*([^*]|/*+[^*//])*/*+//", L"vczh/*is*/genius", 4, 6);
		RunPureInterpretor(L"///*([^*]|/*+[^*//])*/*+//", L"vczh/***is***/genius", 4, 10);
		RunPureInterpretor(L"///*([^*]|/*+[^*//])*/*+//", L"vczh is genius", -1, 0);
	}
}