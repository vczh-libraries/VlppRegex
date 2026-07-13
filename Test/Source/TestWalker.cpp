#include "../../Source/Regex/Regex.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::regex;

template<typename T>
void AssertDetailedScalar(
	RegexLexerWalker_<T>& walker,
	const ObjectString<T>& scalar,
	vint& state,
	bool expectedInvalidState,
	vint expectedToken,
	bool expectedFinalState,
	bool expectedPreviousTokenStop,
	vint expectedRelatedToken
)
{
	vint previousState = state;
	for (vint i = 0; i < scalar.Length(); i++)
	{
		vint token = 100;
		bool finalState = true;
		bool previousTokenStop = true;
		walker.Walk(scalar[i], state, token, finalState, previousTokenStop);
		if (i + 1 < scalar.Length())
		{
			TEST_ASSERT(state == previousState);
			TEST_ASSERT(token == -1);
			TEST_ASSERT(finalState == false);
			TEST_ASSERT(previousTokenStop == false);
		}
		else
		{
			TEST_ASSERT((state == -1) == expectedInvalidState);
			TEST_ASSERT(token == expectedToken);
			TEST_ASSERT(finalState == expectedFinalState);
			TEST_ASSERT(previousTokenStop == expectedPreviousTokenStop);
			TEST_ASSERT(walker.GetRelatedToken(state) == expectedRelatedToken);
		}
	}
}

template<typename T>
vint AssertSimpleScalar(RegexLexerWalker_<T>& walker, const ObjectString<T>& scalar, vint state)
{
	for (vint i = 0; i < scalar.Length(); i++)
	{
		vint previousState = state;
		state = walker.Walk(scalar[i], state);
		if (i + 1 < scalar.Length())
		{
			TEST_ASSERT(state == previousState);
		}
	}
	return state;
}

template<typename T>
void AssertUnicodeWalker(RegexLexer& wordLexer, RegexLexer& scalarLexer, RegexLexer& inverseWordLexer, const ObjectString<T>& scalar, const ObjectString<T>& scalarAndWord)
{
	{
		auto walker = wordLexer.Walk<T>();
		auto state = walker.GetStartState();
		AssertDetailedScalar(walker, scalar, state, true, -1, true, true, -1);

		auto simpleWalker = wordLexer.Walk<T>();
		state = AssertSimpleScalar(simpleWalker, scalar, simpleWalker.GetStartState());
		TEST_ASSERT(state == -1);
	}
	{
		auto walker = scalarLexer.Walk<T>();
		auto state = walker.GetStartState();
		AssertDetailedScalar(walker, scalar, state, false, 0, true, false, 0);

		vint token = -1;
		bool finalState = false;
		bool previousTokenStop = false;
		walker.Walk(static_cast<T>(U'A'), state, token, finalState, previousTokenStop);
		TEST_ASSERT(token == 1);
		TEST_ASSERT(finalState == true);
		TEST_ASSERT(previousTokenStop == true);
		TEST_ASSERT(walker.GetRelatedToken(state) == 1);

		auto simpleWalker = scalarLexer.Walk<T>();
		state = AssertSimpleScalar(simpleWalker, scalar, simpleWalker.GetStartState());
		TEST_ASSERT(walker.GetRelatedToken(state) == 0);
		state = simpleWalker.Walk(static_cast<T>(U'A'), state);
		TEST_ASSERT(walker.GetRelatedToken(state) == 1);
	}
	{
		auto walker = scalarLexer.Walk<T>();
		auto state = walker.GetStartState();
		state = walker.Walk(static_cast<T>(U'A'), state);
		TEST_ASSERT(walker.GetRelatedToken(state) == 1);
		AssertDetailedScalar(walker, scalar, state, false, 0, true, true, 0);

		auto simpleWalker = scalarLexer.Walk<T>();
		state = simpleWalker.Walk(static_cast<T>(U'A'), simpleWalker.GetStartState());
		TEST_ASSERT(simpleWalker.GetRelatedToken(state) == 1);
		state = AssertSimpleScalar(simpleWalker, scalar, state);
		TEST_ASSERT(simpleWalker.GetRelatedToken(state) == 0);
	}
	{
		auto walker = scalarLexer.Walk<T>();
		vint state = -1;
		AssertDetailedScalar(walker, scalar, state, false, 0, true, true, 0);
	}
	if (scalar.Length() > 1)
	{
		auto walker = scalarLexer.Walk<T>();
		vint state = walker.GetStartState();
		for (vint i = 0; i + 1 < scalar.Length(); i++)
		{
			vint token = 100;
			bool finalState = true;
			bool previousTokenStop = true;
			vint previousState = state;
			walker.Walk(scalar[i], state, token, finalState, previousTokenStop);
			TEST_ASSERT(state == previousState);
			TEST_ASSERT(token == -1);
			TEST_ASSERT(finalState == false);
			TEST_ASSERT(previousTokenStop == false);
		}

		auto copiedWalker = walker;
		vint copiedState = state;
		AssertDetailedScalar(walker, ObjectString<T>::FromChar(scalar[scalar.Length() - 1]), state, false, 0, true, false, 0);
		AssertDetailedScalar(copiedWalker, ObjectString<T>::FromChar(scalar[scalar.Length() - 1]), copiedState, false, 0, true, false, 0);
	}
	{
		auto walker = inverseWordLexer.Walk<T>();
		auto state = walker.GetStartState();
		AssertDetailedScalar(walker, scalar, state, false, 0, true, false, 0);

		auto simpleWalker = inverseWordLexer.Walk<T>();
		state = AssertSimpleScalar(simpleWalker, scalar, simpleWalker.GetStartState());
		TEST_ASSERT(walker.GetRelatedToken(state) == 0);
	}

	Array<T> boundedScalar(scalar.Buffer(), scalar.Length());
	Array<T> boundedScalarAndWord(scalarAndWord.Buffer(), scalarAndWord.Length());
	{
		auto walker = wordLexer.Walk<T>();
		TEST_ASSERT(walker.IsClosedToken(&boundedScalar[0], boundedScalar.Count()) == true);
		TEST_ASSERT(walker.IsClosedToken(scalar) == true);
	}
	{
		auto walker = scalarLexer.Walk<T>();
		TEST_ASSERT(walker.IsClosedToken(&boundedScalar[0], boundedScalar.Count()) == false);
		TEST_ASSERT(walker.IsClosedToken(&boundedScalarAndWord[0], scalar.Length()) == false);
		TEST_ASSERT(walker.IsClosedToken(scalar) == false);
		TEST_ASSERT(walker.IsClosedToken(&boundedScalarAndWord[0], boundedScalarAndWord.Count()) == true);
		TEST_ASSERT(walker.IsClosedToken(scalarAndWord) == true);
	}
	{
		auto walker = inverseWordLexer.Walk<T>();
		TEST_ASSERT(walker.IsClosedToken(&boundedScalar[0], boundedScalar.Count()) == false);
		TEST_ASSERT(walker.IsClosedToken(scalar) == false);
	}
}

TEST_FILE
{
#define WALK(INPUT, TOKEN, RESULT, STOP)\
	do\
	{\
		vint token=-1;\
		bool finalState=false;\
		bool previousTokenStop=false;\
		walker.Walk((INPUT), state, token, finalState, previousTokenStop);\
		TEST_ASSERT((TOKEN)==token);\
		TEST_ASSERT(RESULT==finalState);\
		TEST_ASSERT(STOP==previousTokenStop);\
	}while(0)\

	TEST_CASE(L"Test RegexLexerWalker")
	{
		List<WString> codes;
		codes.Add(L"/d+(./d+)?");
		codes.Add(L"[a-zA-Z_]/w*");
		codes.Add(L"\"[^\"]*\"");
		RegexLexer lexer(codes);
		RegexLexerWalker walker = lexer.Walk();

		vint state = -1;

		WALK(L' ', -1, true, true);
		WALK(L'g', 1, true, true);
		WALK(L'e', 1, true, false);
		WALK(L'n', 1, true, false);
		WALK(L'i', 1, true, false);
		WALK(L'u', 1, true, false);
		WALK(L's', 1, true, false);

		WALK(L' ', -1, true, true);

		WALK(L'1', 0, true, true);
		WALK(L'0', 0, true, false);
		WALK(L'.', -1, false, false);

		WALK(L'.', -1, true, true);

		WALK(L'1', 0, true, true);
		WALK(L'0', 0, true, false);
		WALK(L'.', -1, false, false);
		WALK(L'1', 0, true, false);
		WALK(L'0', 0, true, false);

		WALK(L' ', -1, true, true);
		WALK(L' ', -1, true, true);
		WALK(L' ', -1, true, true);

		WALK(L'\"', -1, false, true);
		WALK(L'\"', 2, true, false);

		WALK(L'\"', -1, false, true);
		WALK(L'g', -1, false, false);
		WALK(L'e', -1, false, false);
		WALK(L'n', -1, false, false);
		WALK(L'i', -1, false, false);
		WALK(L'u', -1, false, false);
		WALK(L's', -1, false, false);
		WALK(L'\"', 2, true, false);
});

	TEST_CASE(L"Test RegexLexerWalker with Unicode scalars")
	{
		List<WString> wordCodes;
		wordCodes.Add(L"/w+");
		RegexLexer wordLexer(wordCodes);

		List<WString> scalarCodes;
		scalarCodes.Add(L"[𦁚]+");
		scalarCodes.Add(L"/w+");
		RegexLexer scalarLexer(scalarCodes);

		List<WString> inverseWordCodes;
		inverseWordCodes.Add(L"/W+");
		RegexLexer inverseWordLexer(inverseWordCodes);

		AssertUnicodeWalker(wordLexer, scalarLexer, inverseWordLexer, WString(L"𦁚"), WString(L"𦁚A"));
		AssertUnicodeWalker(wordLexer, scalarLexer, inverseWordLexer, U8String(u8"𦁚"), U8String(u8"𦁚A"));
		AssertUnicodeWalker(wordLexer, scalarLexer, inverseWordLexer, U16String(u"𦁚"), U16String(u"𦁚A"));
		AssertUnicodeWalker(wordLexer, scalarLexer, inverseWordLexer, U32String(U"𦁚"), U32String(U"𦁚A"));
	});

#undef WALK
}
