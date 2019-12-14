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
		RegexLexer lexer(codes, {});
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

#undef WALK
}