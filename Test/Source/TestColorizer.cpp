#include "ColorizerCommon.h"

TEST_FILE
{
	TEST_CASE(L"Test RegexLexerColorizer 1")
	{
		List<U32String> codes;
		codes.Add(U"/d+(./d+)?");
		codes.Add(U"[a-zA-Z_]/w*");
		codes.Add(U"\"[^\"]*\"");

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

	TEST_CASE(L"Test RegexLexerColorizer 2")
	{
		List<U32String> codes;
		codes.Add(U"/w+");
		codes.Add(U"\"[^\"]*\"");
		codes.Add(U"/s+");

		vint colors[100];
		RegexProc proc;
		proc.colorizeProc = &ColorizerProc;
		proc.argument = colors;
		RegexLexer lexer(codes, proc);
		RegexLexerColorizer colorizer = lexer.Colorize();

		colorizer.Pass(L'\r');
		colorizer.Pass(L'\n');
		{
			const wchar_t input[] = L"\"text\"";
			vint expect[] = { 1, 1, 1, 1, 1, 1 };
			TEST_ASSERT(AssertColorizer(colors, expect, colorizer, input, true) == nullptr);
		}
	});
}