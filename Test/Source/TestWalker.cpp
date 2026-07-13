#include "../../Source/Regex/Regex.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::regex;

template<typename T>
void AssertUnicodeWalker(RegexLexer& wordLexer, RegexLexer& scalarLexer, RegexLexer& inverseWordLexer, const ObjectString<T>& scalar, const ObjectString<T>& scalarAndWord)
{
	{
		auto walker = wordLexer.Walk<T>();
		auto state = walker.GetStartState();
		vint token = -1;
		bool finalState = false;
		bool previousTokenStop = false;
		walker.Walk(U'𦁚', state, token, finalState, previousTokenStop);
		TEST_ASSERT(state == -1);
		TEST_ASSERT(token == -1);
		TEST_ASSERT(finalState == true);
		TEST_ASSERT(previousTokenStop == true);
		TEST_ASSERT(walker.GetRelatedToken(state) == -1);
		TEST_ASSERT(walker.Walk(U'𦁚', walker.GetStartState()) == -1);
	}
	{
		auto walker = scalarLexer.Walk<T>();
		auto state = walker.GetStartState();
		vint token = -1;
		bool finalState = false;
		bool previousTokenStop = false;
		walker.Walk(U'𦁚', state, token, finalState, previousTokenStop);
		TEST_ASSERT(token == 0);
		TEST_ASSERT(finalState == true);
		TEST_ASSERT(previousTokenStop == false);
		TEST_ASSERT(walker.GetRelatedToken(state) == 0);

		walker.Walk(U'A', state, token, finalState, previousTokenStop);
		TEST_ASSERT(token == 1);
		TEST_ASSERT(finalState == true);
		TEST_ASSERT(previousTokenStop == true);
		TEST_ASSERT(walker.GetRelatedToken(state) == 1);

		state = walker.Walk(U'𦁚', walker.GetStartState());
		TEST_ASSERT(walker.GetRelatedToken(state) == 0);
		state = walker.Walk(U'A', state);
		TEST_ASSERT(walker.GetRelatedToken(state) == 1);
	}
	{
		auto walker = inverseWordLexer.Walk<T>();
		auto state = walker.GetStartState();
		vint token = -1;
		bool finalState = false;
		bool previousTokenStop = false;
		walker.Walk(U'𦁚', state, token, finalState, previousTokenStop);
		TEST_ASSERT(token == 0);
		TEST_ASSERT(finalState == true);
		TEST_ASSERT(previousTokenStop == false);
		TEST_ASSERT(walker.GetRelatedToken(state) == 0);

		state = walker.Walk(U'𦁚', walker.GetStartState());
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
