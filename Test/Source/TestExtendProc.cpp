#include "ColorizerCommon.h"

struct TestExtendProc_InterTokenState
{
	WString postfix;
};

void TestExtendProc_Deleter(void* interStateDeleter)
{
	delete (TestExtendProc_InterTokenState*)interStateDeleter;
}

void TestExtendProc_ExtendProc(void* argument, const wchar_t* reading, vint length, bool completeText, RegexProcessingToken& processingToken)
{
	WString readingBuffer = length == -1 ? WString::Unmanaged(reading) : WString::CopyFrom(reading, length);
	reading = readingBuffer.Buffer();

	if (processingToken.token == 2 || processingToken.token == 4)
	{
		WString postfix;
		if (processingToken.interTokenState)
		{
			postfix = ((TestExtendProc_InterTokenState*)processingToken.interTokenState)->postfix;
		}
		else
		{
			postfix = L")" + WString::CopyFrom(reading + 2, processingToken.length - 3) + L"\"";
		}

		auto find = wcsstr(reading, postfix.Buffer());
		if (find)
		{
			processingToken.length = (vint)(find - reading) + postfix.Length();
			processingToken.completeToken = true;
			processingToken.interTokenState = nullptr;
		}
		else
		{
			processingToken.length = readingBuffer.Length();
			processingToken.token = 4;
			processingToken.completeToken = false;

			if (!completeText && !processingToken.interTokenState)
			{
				auto state = new TestExtendProc_InterTokenState;
				state->postfix = postfix;
				processingToken.interTokenState = state;
			}
		}
	}
}

TEST_FILE
{
	auto TestRegexLexer6Validation = [](List<RegexToken>& tokens)
	{
		TEST_ASSERT(tokens.Count() == 10);
		//[123]
		TEST_ASSERT(tokens[0].start == 0);
		TEST_ASSERT(tokens[0].length == 3);
		TEST_ASSERT(tokens[0].token == 0);
		TEST_ASSERT(tokens[0].rowStart == 0);
		TEST_ASSERT(tokens[0].columnStart == 0);
		TEST_ASSERT(tokens[0].rowEnd == 0);
		TEST_ASSERT(tokens[0].columnEnd == 2);
		TEST_ASSERT(tokens[0].completeToken == true);
		//[ ]
		TEST_ASSERT(tokens[1].start == 3);
		TEST_ASSERT(tokens[1].length == 1);
		TEST_ASSERT(tokens[1].token == -1);
		TEST_ASSERT(tokens[1].rowStart == 0);
		TEST_ASSERT(tokens[1].columnStart == 3);
		TEST_ASSERT(tokens[1].rowEnd == 0);
		TEST_ASSERT(tokens[1].columnEnd == 3);
		TEST_ASSERT(tokens[1].completeToken == true);
		//[456]
		TEST_ASSERT(tokens[2].start == 4);
		TEST_ASSERT(tokens[2].length == 3);
		TEST_ASSERT(tokens[2].token == 0);
		TEST_ASSERT(tokens[2].rowStart == 0);
		TEST_ASSERT(tokens[2].columnStart == 4);
		TEST_ASSERT(tokens[2].rowEnd == 0);
		TEST_ASSERT(tokens[2].columnEnd == 6);
		TEST_ASSERT(tokens[2].completeToken == true);
		//[\n]
		TEST_ASSERT(tokens[3].start == 7);
		TEST_ASSERT(tokens[3].length == 1);
		TEST_ASSERT(tokens[3].token == -1);
		TEST_ASSERT(tokens[3].rowStart == 0);
		TEST_ASSERT(tokens[3].columnStart == 7);
		TEST_ASSERT(tokens[3].rowEnd == 0);
		TEST_ASSERT(tokens[3].columnEnd == 7);
		TEST_ASSERT(tokens[3].completeToken == true);
		//[simple text]
		TEST_ASSERT(tokens[4].start == 8);
		TEST_ASSERT(tokens[4].length == 13);
		TEST_ASSERT(tokens[4].token == 1);
		TEST_ASSERT(tokens[4].rowStart == 1);
		TEST_ASSERT(tokens[4].columnStart == 0);
		TEST_ASSERT(tokens[4].rowEnd == 1);
		TEST_ASSERT(tokens[4].columnEnd == 12);
		TEST_ASSERT(tokens[4].completeToken == true);
		//[\n]
		TEST_ASSERT(tokens[5].start == 21);
		TEST_ASSERT(tokens[5].length == 1);
		TEST_ASSERT(tokens[5].token == -1);
		TEST_ASSERT(tokens[5].rowStart == 1);
		TEST_ASSERT(tokens[5].columnStart == 13);
		TEST_ASSERT(tokens[5].rowEnd == 1);
		TEST_ASSERT(tokens[5].columnEnd == 13);
		TEST_ASSERT(tokens[5].completeToken == true);
		//[123]
		TEST_ASSERT(tokens[6].start == 22);
		TEST_ASSERT(tokens[6].length == 3);
		TEST_ASSERT(tokens[6].token == 0);
		TEST_ASSERT(tokens[6].rowStart == 2);
		TEST_ASSERT(tokens[6].columnStart == 0);
		TEST_ASSERT(tokens[6].rowEnd == 2);
		TEST_ASSERT(tokens[6].columnEnd == 2);
		TEST_ASSERT(tokens[6].completeToken == true);
		//["$===(+\nabcde\n-)==="]
		TEST_ASSERT(tokens[7].start == 25);
		TEST_ASSERT(tokens[7].length == 20);
		TEST_ASSERT(tokens[7].token == 2);
		TEST_ASSERT(tokens[7].rowStart == 2);
		TEST_ASSERT(tokens[7].columnStart == 3);
		TEST_ASSERT(tokens[7].rowEnd == 4);
		TEST_ASSERT(tokens[7].columnEnd == 5);
		TEST_ASSERT(tokens[7].completeToken == true);
		//[456]
		TEST_ASSERT(tokens[8].start == 45);
		TEST_ASSERT(tokens[8].length == 3);
		TEST_ASSERT(tokens[8].token == 0);
		TEST_ASSERT(tokens[8].rowStart == 4);
		TEST_ASSERT(tokens[8].columnStart == 6);
		TEST_ASSERT(tokens[8].rowEnd == 4);
		TEST_ASSERT(tokens[8].columnEnd == 8);
		TEST_ASSERT(tokens[8].completeToken == true);
		//[$"===(]
		TEST_ASSERT(tokens[9].start == 48);
		TEST_ASSERT(tokens[9].length == 6);
		TEST_ASSERT(tokens[9].token == 4);
		TEST_ASSERT(tokens[9].rowStart == 4);
		TEST_ASSERT(tokens[9].columnStart == 9);
		TEST_ASSERT(tokens[9].rowEnd == 4);
		TEST_ASSERT(tokens[9].columnEnd == 14);
		TEST_ASSERT(tokens[9].completeToken == false);
	};

	TEST_CASE(L"Test RegexLexer with ExtendProc")
	{
		List<WString> codes;
		codes.Add(L"/d+");
		codes.Add(L"\"[^\"]*\"");
		codes.Add(L"/$\"=*/(");
		codes.Add(L"/t+");

		RegexProc proc;
		proc.deleter = TestExtendProc_Deleter;
		proc.extendProc = TestExtendProc_ExtendProc;
		RegexLexer lexer(codes, proc);

		WString input = LR"test_input(123 456
"simple text"
123$"===(+
abcde
-)==="456$"===()test_input";
		{
			List<RegexToken> tokens;
			CopyFrom(tokens, lexer.Parse(input));
			TestRegexLexer6Validation(tokens);
		}
		{
			List<RegexToken> tokens;
			lexer.Parse(input).ReadToEnd(tokens);
			TestRegexLexer6Validation(tokens);
		}
	});

	TEST_CASE(L"Test RegexLexerColorizer with ExtendProc")
	{
		List<WString> codes;
		codes.Add(L"/d+");
		codes.Add(L"\"[^\"]*\"");
		codes.Add(L"/$\"=*/(");
		codes.Add(L"/s+");

		vint colors[100];
		RegexProc proc;
		proc.deleter = &TestExtendProc_Deleter;
		proc.extendProc = &TestExtendProc_ExtendProc;
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
			vint expect[] = { 0,0,0,4,4,4,4,4,4,4 };
			auto state = AssertColorizer(colors, expect, colorizer, input, true);
			TEST_ASSERT(state != nullptr);
			lastInterTokenState = state;
		}
		colorizer.Pass(L'\r');
		colorizer.Pass(L'\n');
		{
			const wchar_t input[] = L"abcde";
			vint expect[] = { 4,4,4,4,4 };
			auto state = AssertColorizer(colors, expect, colorizer, input, true);
			TEST_ASSERT(state == lastInterTokenState);
			lastInterTokenState = state;
		}
		colorizer.Pass(L'\r');
		colorizer.Pass(L'\n');
		{
			const wchar_t input[] = L"-)===\"456$\"===(";
			vint expect[] = { 4,4,4,4,4,4,0,0,0,4,4,4,4,4,4 };
			auto state = AssertColorizer(colors, expect, colorizer, input, true);
			TEST_ASSERT(state != nullptr && state != lastInterTokenState);
			proc.deleter(lastInterTokenState);
			lastInterTokenState = state;
		}

		if (lastInterTokenState)
		{
			proc.deleter(lastInterTokenState);
		}
	});
}