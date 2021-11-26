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
	void EnsureAutomatonIdentical(Automaton::Ref a1, Automaton::Ref a2)
	{
		TEST_ASSERT(a1->states.Count() == a2->states.Count());
		TEST_ASSERT(a1->transitions.Count() == a2->transitions.Count());
		TEST_ASSERT(CompareEnumerable(a1->captureNames, a2->captureNames) == 0);
		TEST_ASSERT(a1->states.IndexOf(a1->startState) == a2->states.IndexOf(a2->startState));

		for (vint i = 0; i < a1->states.Count(); i++)
		{
			auto s1 = a1->states[i].Obj();
			auto s2 = a2->states[i].Obj();

			TEST_ASSERT(s1->finalState == s2->finalState);
			TEST_ASSERT(s1->transitions.Count() == s2->transitions.Count());
			TEST_ASSERT(s1->inputs.Count() == s2->inputs.Count());

			for (vint j = 0; j < s1->transitions.Count(); j++)
			{
				auto t1 = s1->transitions[j];
				auto t2 = s2->transitions[j];
				TEST_ASSERT(a1->transitions.IndexOf(t1) == a2->transitions.IndexOf(t2));
			}

			for (vint j = 0; j < s1->inputs.Count(); j++)
			{
				auto t1 = s1->inputs[j];
				auto t2 = s2->inputs[j];
				TEST_ASSERT(a1->transitions.IndexOf(t1) == a2->transitions.IndexOf(t2));
			}
		}

		for (vint i = 0; i < a1->transitions.Count(); i++)
		{
			auto t1 = a1->transitions[i].Obj();
			auto t2 = a2->transitions[i].Obj();

			TEST_ASSERT(a1->states.IndexOf(t1->source) == a2->states.IndexOf(t2->source));
			TEST_ASSERT(a1->states.IndexOf(t1->target) == a2->states.IndexOf(t2->target));
			TEST_ASSERT(t1->range == t2->range);
			TEST_ASSERT(t1->type == t2->type);
			TEST_ASSERT(t1->capture == t2->capture);
			TEST_ASSERT(t1->index == t2->index);
		}
	}

	void EnsurePureInterpretorIdentical(PureInterpretor* p1, PureInterpretor* p2)
	{
		TEST_ASSERT(p1->stateCount == p2->stateCount);
		TEST_ASSERT(p1->charSetCount == p2->charSetCount);
		TEST_ASSERT(p1->startState == p2->startState);
		TEST_ASSERT(memcmp(p1->charMap, p2->charMap, sizeof(vint) * PureInterpretor::SupportedCharCount) == 0);
		TEST_ASSERT(memcmp(p1->transitions, p2->transitions, sizeof(vint) * (p1->stateCount * p1->charSetCount)) == 0);
		TEST_ASSERT(memcmp(p1->finalState, p2->finalState, sizeof(bool) * p1->stateCount) == 0);
	}

	Ptr<PureInterpretor> BuildPureInterpretor(
		const char32_t* code,
		Automaton::Ref& eNfa,
		Automaton::Ref& nfa,
		Automaton::Ref& dfa)
	{
		CharRange::List subsets;
		Dictionary<State*, State*> nfaStateMap;
		Group<State*, State*> dfaStateMap;

		RegexExpression::Ref regex = ParseRegexExpression(code);
		Expression::Ref expression = regex->Merge();
		expression->NormalizeCharSet(subsets);
		eNfa = expression->GenerateEpsilonNfa();
		nfa = EpsilonNfaToNfa(eNfa, PureEpsilonChecker, nfaStateMap);
		dfa = NfaToDfa(nfa, dfaStateMap);
		return new PureInterpretor(dfa, subsets);
	}

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
		Automaton::Ref eNfa, nfa, dfa;
		auto interpretor = BuildPureInterpretor(code, eNfa, nfa, dfa);
		TEST_CASE(u32tow(code) + WString(L" on ") + input)
		{
			for (vint i = 0; i < 10; i++)
			{
				Automaton::Ref eNfa2, nfa2, dfa2;
				auto second = BuildPureInterpretor(code, eNfa2, nfa2, dfa2);
				EnsureAutomatonIdentical(eNfa, eNfa2);
				EnsureAutomatonIdentical(nfa, nfa2);
				EnsureAutomatonIdentical(dfa, dfa2);
				EnsurePureInterpretorIdentical(interpretor.Obj(), second.Obj());
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