#include "ColorizerCommon.h"

struct ColorizerRecord
{
	vint										start;
	vint										length;
	vint										token;
};

struct UnicodeColorizerArgument
{
	vint*										colors;
	vint*										writes;
	vint										length;
	ColorizerRecord*							records;
	vint										recordCount;
	vint										recordCapacity;
};

void UnicodeColorizerProc(void* argument, vint start, vint length, vint token)
{
	auto data = (UnicodeColorizerArgument*)argument;
	TEST_ASSERT(0 <= start && 0 < length && start + length <= data->length);
	TEST_ASSERT(data->recordCount < data->recordCapacity);
	data->records[data->recordCount++] =
	{
		.start = start,
		.length = length,
		.token = token,
	};
	for (vint i = 0; i < length; i++)
	{
		data->colors[start + i] = token;
		data->writes[start + i]++;
	}
}

template<typename T>
void AssertSingleUnicodeColorizer(RegexLexer& lexer, const ObjectString<T>& input, vint expectedToken)
{
	Array<T> boundedInput(input.Buffer(), input.Length());
	vint colors[100];
	vint writes[100];
	ColorizerRecord records[100];
	for (vint i = 0; i < 100; i++)
	{
		colors[i] = -2;
		writes[i] = 0;
	}
	UnicodeColorizerArgument argument =
	{
		.colors = colors,
		.writes = writes,
		.length = boundedInput.Count(),
		.records = records,
		.recordCount = 0,
		.recordCapacity = 100,
	};
	RegexProc_<T> proc;
	proc.colorizeProc = &UnicodeColorizerProc;
	proc.argument = &argument;
	auto colorizer = lexer.Colorize(proc);

	TEST_ASSERT(colorizer.Colorize(&boundedInput[0], boundedInput.Count()) == nullptr);
	TEST_ASSERT(argument.recordCount == 1);
	TEST_ASSERT(records[0].start == 0);
	TEST_ASSERT(records[0].length == boundedInput.Count());
	TEST_ASSERT(records[0].token == expectedToken);
	for (vint i = 0; i < 100; i++)
	{
		if (i < boundedInput.Count())
		{
			TEST_ASSERT(colors[i] == expectedToken);
			TEST_ASSERT(writes[i] == 1);
		}
		else
		{
			TEST_ASSERT(colors[i] == -2);
			TEST_ASSERT(writes[i] == 0);
		}
	}
}

template<typename T>
void AssertFocusedUnicodeColorizer(RegexLexer& lexer, const ObjectString<T>& input, vint expectedStart, vint expectedLength)
{
	Array<T> boundedInput(input.Buffer(), input.Length());
	vint colors[100];
	vint writes[100];
	ColorizerRecord records[100];
	for (vint i = 0; i < 100; i++)
	{
		colors[i] = -2;
		writes[i] = 0;
	}
	UnicodeColorizerArgument argument =
	{
		.colors = colors,
		.writes = writes,
		.length = boundedInput.Count(),
		.records = records,
		.recordCount = 0,
		.recordCapacity = 100,
	};
	RegexProc_<T> proc;
	proc.colorizeProc = &UnicodeColorizerProc;
	proc.argument = &argument;
	auto colorizer = lexer.Colorize(proc);

	TEST_ASSERT(colorizer.Colorize(&boundedInput[0], boundedInput.Count()) == nullptr);
	vint covered = 0;
	vint recognized = 0;
	for (vint i = 0; i < argument.recordCount; i++)
	{
		TEST_ASSERT(records[i].start == covered);
		covered += records[i].length;
		if (records[i].token == 0)
		{
			recognized++;
			TEST_ASSERT(records[i].start == expectedStart);
			TEST_ASSERT(records[i].length == expectedLength);
		}
	}
	TEST_ASSERT(covered == boundedInput.Count());
	TEST_ASSERT(recognized == 1);
	for (vint i = 0; i < 100; i++)
	{
		if (i < boundedInput.Count())
		{
			vint expectedToken = expectedStart <= i && i < expectedStart + expectedLength ? 0 : -1;
			TEST_ASSERT(colors[i] == expectedToken);
			TEST_ASSERT(writes[i] == 1);
		}
		else
		{
			TEST_ASSERT(colors[i] == -2);
			TEST_ASSERT(writes[i] == 0);
		}
	}
}

struct PassColorizerArgument
{
	vint										count;
	ColorizerRecord							records[2];
};

void PassColorizerProc(void* argument, vint start, vint length, vint token)
{
	auto data = (PassColorizerArgument*)argument;
	TEST_ASSERT(data->count < 2);
	data->records[data->count++] =
	{
		.start = start,
		.length = length,
		.token = token,
	};
}

template<typename T>
struct PassExtendArgument
{
	vint										extendCount;
	vint										colorizeCount;
	T										reading[6];
	vint										length;
	bool										completeText;
	vint										tokenStart;
	vint										tokenLength;
	vint										token;
	bool										completeToken;
	void*										interTokenState;
};

template<typename T>
void PassExtendProc(void* argument, const T* reading, vint length, bool completeText, RegexProcessingToken& processingToken)
{
	auto data = (PassExtendArgument<T>*)argument;
	TEST_ASSERT(0 <= length && length <= 6);
	data->extendCount++;
	data->length = length;
	data->completeText = completeText;
	data->tokenStart = processingToken.start;
	data->tokenLength = processingToken.length;
	data->token = processingToken.token;
	data->completeToken = processingToken.completeToken;
	data->interTokenState = processingToken.interTokenState;
	for (vint i = 0; i < length; i++)
	{
		data->reading[i] = reading[i];
	}
}

template<typename T>
void PassExtendColorizerProc(void* argument, vint start, vint length, vint token)
{
	auto data = (PassExtendArgument<T>*)argument;
	data->colorizeCount++;
}

template<typename T>
void AssertUnicodeColorizerPass(RegexLexer& sequenceLexer, RegexLexer& scalarLexer, const ObjectString<T>& prefix, const ObjectString<T>& suffix, const ObjectString<T>& encodedScalar)
{
	{
		PassColorizerArgument argument = {};
		RegexProc_<T> proc;
		proc.colorizeProc = &PassColorizerProc;
		proc.argument = &argument;
		auto colorizer = sequenceLexer.Colorize(proc);
		Array<T> boundedPrefix(prefix.Buffer(), prefix.Length());
		Array<T> boundedSuffix(suffix.Buffer(), suffix.Length());

		TEST_ASSERT(colorizer.Colorize(&boundedPrefix[0], boundedPrefix.Count()) == nullptr);
		TEST_ASSERT(argument.count == 1);
		colorizer.Pass(U'𦁚');
		TEST_ASSERT(argument.count == 1);
		TEST_ASSERT(colorizer.Colorize(&boundedSuffix[0], boundedSuffix.Count()) == nullptr);
		TEST_ASSERT(argument.count == 2);
		TEST_ASSERT(argument.records[0].start == 0);
		TEST_ASSERT(argument.records[0].length == 1);
		TEST_ASSERT(argument.records[0].token == 0);
		TEST_ASSERT(argument.records[1].start == 0);
		TEST_ASSERT(argument.records[1].length == 1);
		TEST_ASSERT(argument.records[1].token == 0);
	}
	{
		PassExtendArgument<T> argument = {};
		RegexProc_<T> proc;
		proc.extendProc = &PassExtendProc<T>;
		proc.colorizeProc = &PassExtendColorizerProc<T>;
		proc.argument = &argument;
		auto colorizer = scalarLexer.Colorize(proc);

		colorizer.Pass(U'𦁚');
		TEST_ASSERT(argument.extendCount == 1);
		TEST_ASSERT(argument.colorizeCount == 0);
		TEST_ASSERT(argument.length == encodedScalar.Length());
		TEST_ASSERT(argument.completeText == false);
		TEST_ASSERT(argument.tokenStart == 0);
		TEST_ASSERT(argument.tokenLength == encodedScalar.Length());
		TEST_ASSERT(argument.token == 0);
		TEST_ASSERT(argument.completeToken == true);
		TEST_ASSERT(argument.interTokenState == nullptr);
		for (vint i = 0; i < encodedScalar.Length(); i++)
		{
			TEST_ASSERT(argument.reading[i] == encodedScalar[i]);
		}
	}
}

TEST_FILE
{
	TEST_CASE(L"Test RegexLexerColorizer 1")
	{
		List<WString> codes;
		codes.Add(L"/d+(./d+)?");
		codes.Add(L"[a-zA-Z_]/w*");
		codes.Add(L"\"[^\"]*\"");

		vint colors[100];
		RegexProc proc;
		proc.colorizeProc = &ColorizerProc;
		proc.argument = colors;
		RegexLexer lexer(codes);
		RegexLexerColorizer colorizer = lexer.Colorize(proc);

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
		List<WString> codes;
		codes.Add(L"/w+");
		codes.Add(L"\"[^\"]*\"");
		codes.Add(L"/s+");

		vint colors[100];
		RegexProc proc;
		proc.colorizeProc = &ColorizerProc;
		proc.argument = colors;
		RegexLexer lexer(codes);
		RegexLexerColorizer colorizer = lexer.Colorize(proc);

		colorizer.Pass(L'\r');
		colorizer.Pass(L'\n');
		{
			const wchar_t input[] = L"\"text\"";
			vint expect[] = { 1, 1, 1, 1, 1, 1 };
			TEST_ASSERT(AssertColorizer(colors, expect, colorizer, input, true) == nullptr);
		}
	});

	TEST_CASE(L"Test RegexLexerColorizer with Unicode scalars")
	{
		List<WString> wordCodes;
		wordCodes.Add(L"/w+");
		RegexLexer wordLexer(wordCodes);

		List<WString> scalarCodes;
		scalarCodes.Add(L"[𦁚]+");
		RegexLexer scalarLexer(scalarCodes);

		List<WString> inverseWordCodes;
		inverseWordCodes.Add(L"/W+");
		RegexLexer inverseWordLexer(inverseWordCodes);

		AssertSingleUnicodeColorizer(wordLexer, WString(L"𦁚"), -1);
		AssertSingleUnicodeColorizer(scalarLexer, WString(L"𦁚"), 0);
		AssertSingleUnicodeColorizer(inverseWordLexer, WString(L"𦁚"), 0);
		AssertSingleUnicodeColorizer(wordLexer, U8String(u8"𦁚"), -1);
		AssertSingleUnicodeColorizer(scalarLexer, U8String(u8"𦁚"), 0);
		AssertSingleUnicodeColorizer(inverseWordLexer, U8String(u8"𦁚"), 0);
		AssertSingleUnicodeColorizer(wordLexer, U16String(u"𦁚"), -1);
		AssertSingleUnicodeColorizer(scalarLexer, U16String(u"𦁚"), 0);
		AssertSingleUnicodeColorizer(inverseWordLexer, U16String(u"𦁚"), 0);
		AssertSingleUnicodeColorizer(wordLexer, U32String(U"𦁚"), -1);
		AssertSingleUnicodeColorizer(scalarLexer, U32String(U"𦁚"), 0);
		AssertSingleUnicodeColorizer(inverseWordLexer, U32String(U"𦁚"), 0);

		List<WString> focusedCodes;
		focusedCodes.Add(L"[𣂕𣴑𣱳𦁚]+");
		RegexLexer focusedLexer(focusedCodes);
		AssertFocusedUnicodeColorizer(focusedLexer, U8String(u8"𩰪㦲𦰗𠀼 𣂕𣴑𣱳𦁚 Vczh is genius!@我是天才"), 16, 16);
		AssertFocusedUnicodeColorizer(focusedLexer, U16String(u"𩰪㦲𦰗𠀼 𣂕𣴑𣱳𦁚 Vczh is genius!@我是天才"), 8, 8);
		AssertFocusedUnicodeColorizer(focusedLexer, U32String(U"𩰪㦲𦰗𠀼 𣂕𣴑𣱳𦁚 Vczh is genius!@我是天才"), 5, 4);
#if defined VCZH_WCHAR_UTF16
		AssertFocusedUnicodeColorizer(focusedLexer, WString(L"𩰪㦲𦰗𠀼 𣂕𣴑𣱳𦁚 Vczh is genius!@我是天才"), 8, 8);
#elif defined VCZH_WCHAR_UTF32
		AssertFocusedUnicodeColorizer(focusedLexer, WString(L"𩰪㦲𦰗𠀼 𣂕𣴑𣱳𦁚 Vczh is genius!@我是天才"), 5, 4);
#endif
	});

	TEST_CASE(L"Test RegexLexerColorizer::Pass with a Unicode scalar")
	{
		List<WString> sequenceCodes;
		sequenceCodes.Add(L"A𦁚B");
		RegexLexer sequenceLexer(sequenceCodes);

		List<WString> scalarCodes;
		scalarCodes.Add(L"𦁚");
		RegexLexer scalarLexer(scalarCodes);

		AssertUnicodeColorizerPass(sequenceLexer, scalarLexer, WString(L"A"), WString(L"B"), WString(L"𦁚"));
		AssertUnicodeColorizerPass(sequenceLexer, scalarLexer, U8String(u8"A"), U8String(u8"B"), U8String(u8"𦁚"));
		AssertUnicodeColorizerPass(sequenceLexer, scalarLexer, U16String(u"A"), U16String(u"B"), U16String(u"𦁚"));
		AssertUnicodeColorizerPass(sequenceLexer, scalarLexer, U32String(U"A"), U32String(U"B"), U32String(U"𦁚"));
	});
}
