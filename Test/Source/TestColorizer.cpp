#include "../../Source/Regex/Regex.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::regex;

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

TEST_FILE
{
	TEST_CASE(L"Test RegexLexerColorizer")
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
	});
}