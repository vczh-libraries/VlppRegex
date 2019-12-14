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
	void ColorizerProc(void* argument, vint start, vint length, vint token)
	{
		vint* colors = (vint*)argument;
		for (vint i = 0; i < length; i++)
		{
			colors[start + i] = token;
		}
	}

	template<int Size, int Length>
	void* AssertColorizer(vint(&actual)[Size], vint(&expect)[Length], RegexLexerColorizer& colorizer, const wchar_t(&input)[Length + 1], bool firstLine)
	{
		for (vint i = 0; i < Size; i++)
		{
			actual[i] = -2;
		}
		auto newStateObject = colorizer.Colorize(input, Length);
		for (vint i = 0; i < Length; i++)
		{
			TEST_ASSERT(actual[i] == expect[i]);
		}
		return newStateObject;
	}

	TEST_CASE(TestRegexLexerColorizer1)
	{
		List<WString> codes;
		codes.Add(L"/d+(./d+)?");
		codes.Add(L"[a-zA-Z_]/w*");
		codes.Add(L"\"[^\"]*\"");

		vint colors[100];
		RegexProc proc;
		proc.colorizeProc = &ColorizerProc;
		proc.argument = colors;
		RegexLexer lexer(codes, proc);
		RegexLexerColorizer colorizer = lexer.Colorize();

		{
			const wchar_t input[] = L" genius 10..10.10   \"a";
			vint expect[] = { -1, 1, 1, 1, 1, 1, 1, -1, 0, 0, -1, -1, 0, 0, 0, 0, 0, -1, -1, -1, 2, 2 };
			TEST_ASSERT(AssertColorizer(colors, expect, colorizer, input, true) == nullptr);
		}
		colorizer.Pass(L'\r');
		colorizer.Pass(L'\n');
		{
			const wchar_t input[] = L"b\"\"genius\"";
			vint expect[] = { 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 };
			TEST_ASSERT(AssertColorizer(colors, expect, colorizer, input, true) == nullptr);
		}
	}

	TEST_CASE(TestRegexLexerColorizer2)
	{
		List<WString> codes;
		codes.Add(L"/d+");
		codes.Add(L"\"[^\"]*\"");
		codes.Add(L"/$\"=*/(");

		vint colors[100];
		RegexProc proc;
		proc.deleter = &TestRegexLexer6Deleter;
		proc.extendProc = &TestRegexLexer6ExtendProc;
		proc.colorizeProc = &ColorizerProc;
		proc.argument = colors;
		RegexLexer lexer(codes, proc);
		RegexLexerColorizer colorizer = lexer.Colorize();

		void* lastInterTokenState = nullptr;
		{
			const wchar_t input[] = L"123$\"==()==)==\"456";
			vint expect[] = { 0,0,0,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0 };
			auto state = AssertColorizer(colors, expect, colorizer, input, true);
			TEST_ASSERT(state == nullptr);
			lastInterTokenState = state;
		}
		colorizer.Pass(L'\r');
		colorizer.Pass(L'\n');
		{
			const wchar_t input[] = L"\"simple text\"";
			vint expect[] = { 1,1,1,1,1,1,1,1,1,1,1,1,1 };
			auto state = AssertColorizer(colors, expect, colorizer, input, true);
			TEST_ASSERT(state == nullptr);
			lastInterTokenState = state;
		}
		colorizer.Pass(L'\r');
		colorizer.Pass(L'\n');
		{
			const wchar_t input[] = L"123$\"===(+";
			vint expect[] = { 0,0,0,3,3,3,3,3,3,3 };
			auto state = AssertColorizer(colors, expect, colorizer, input, true);
			TEST_ASSERT(state != nullptr);
			lastInterTokenState = state;
		}
		colorizer.Pass(L'\r');
		colorizer.Pass(L'\n');
		{
			const wchar_t input[] = L"abcde";
			vint expect[] = { 3,3,3,3,3 };
			auto state = AssertColorizer(colors, expect, colorizer, input, true);
			TEST_ASSERT(state == lastInterTokenState);
			lastInterTokenState = state;
		}
		colorizer.Pass(L'\r');
		colorizer.Pass(L'\n');
		{
			const wchar_t input[] = L"-)===\"456$\"===(";
			vint expect[] = { 3,3,3,3,3,3,0,0,0,3,3,3,3,3,3 };
			auto state = AssertColorizer(colors, expect, colorizer, input, true);
			TEST_ASSERT(state != nullptr && state != lastInterTokenState);
			proc.deleter(lastInterTokenState);
			lastInterTokenState = state;
		}

		if (lastInterTokenState)
		{
			proc.deleter(lastInterTokenState);
		}
	}
}