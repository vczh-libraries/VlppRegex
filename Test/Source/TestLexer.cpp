#include <VlppOS.h>
#include "../../Source/Regex/Regex.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::regex;

struct UnicodeLexerToken
{
	vint										start;
	vint										length;
	vint										token;
};

template<typename T, size_t Count>
void AssertUnicodeLexer(RegexLexer& lexer, const ObjectString<T>& input, const UnicodeLexerToken(&expected)[Count])
{
	List<RegexToken_<T>> tokens;
	CopyFrom(tokens, lexer.Parse(input));

	TEST_ASSERT(tokens.Count() == (vint)Count);
	for (vint i = 0; i < (vint)Count; i++)
	{
		TEST_ASSERT(tokens[i].start == expected[i].start);
		TEST_ASSERT(tokens[i].length == expected[i].length);
		TEST_ASSERT(tokens[i].token == expected[i].token);
		TEST_ASSERT(tokens[i].rowStart == 0);
		TEST_ASSERT(tokens[i].columnStart == expected[i].start);
		TEST_ASSERT(tokens[i].rowEnd == 0);
		TEST_ASSERT(tokens[i].columnEnd == expected[i].start + expected[i].length - 1);
		TEST_ASSERT(tokens[i].completeToken == true);
	}
}

TEST_FILE
{
	auto TestRegexLexer1Validation = [](List<RegexToken>& tokens)
	{
		TEST_ASSERT(tokens.Count() == 9);
		for (vint i = 0; i < tokens.Count(); i++)
		{
			TEST_ASSERT(tokens[i].completeToken == true);
		}
		//[vczh]
		TEST_ASSERT(tokens[0].start == 0);
		TEST_ASSERT(tokens[0].length == 4);
		TEST_ASSERT(tokens[0].token == 2);
		TEST_ASSERT(tokens[0].rowStart == 0);
		TEST_ASSERT(tokens[0].columnStart == 0);
		TEST_ASSERT(tokens[0].rowEnd == 0);
		TEST_ASSERT(tokens[0].columnEnd == 3);
		//[ ]
		TEST_ASSERT(tokens[1].start == 4);
		TEST_ASSERT(tokens[1].length == 1);
		TEST_ASSERT(tokens[1].token == 1);
		TEST_ASSERT(tokens[1].rowStart == 0);
		TEST_ASSERT(tokens[1].columnStart == 4);
		TEST_ASSERT(tokens[1].rowEnd == 0);
		TEST_ASSERT(tokens[1].columnEnd == 4);
		//[is]
		TEST_ASSERT(tokens[2].start == 5);
		TEST_ASSERT(tokens[2].length == 2);
		TEST_ASSERT(tokens[2].token == 2);
		TEST_ASSERT(tokens[2].rowStart == 0);
		TEST_ASSERT(tokens[2].columnStart == 5);
		TEST_ASSERT(tokens[2].rowEnd == 0);
		TEST_ASSERT(tokens[2].columnEnd == 6);
		//[$$]
		TEST_ASSERT(tokens[3].start == 7);
		TEST_ASSERT(tokens[3].length == 2);
		TEST_ASSERT(tokens[3].token == -1);
		TEST_ASSERT(tokens[3].rowStart == 0);
		TEST_ASSERT(tokens[3].columnStart == 7);
		TEST_ASSERT(tokens[3].rowEnd == 0);
		TEST_ASSERT(tokens[3].columnEnd == 8);
		//[a]
		TEST_ASSERT(tokens[4].start == 9);
		TEST_ASSERT(tokens[4].length == 1);
		TEST_ASSERT(tokens[4].token == 2);
		TEST_ASSERT(tokens[4].rowStart == 0);
		TEST_ASSERT(tokens[4].columnStart == 9);
		TEST_ASSERT(tokens[4].rowEnd == 0);
		TEST_ASSERT(tokens[4].columnEnd == 9);
		//[&&]
		TEST_ASSERT(tokens[5].start == 10);
		TEST_ASSERT(tokens[5].length == 2);
		TEST_ASSERT(tokens[5].token == -1);
		TEST_ASSERT(tokens[5].rowStart == 0);
		TEST_ASSERT(tokens[5].columnStart == 10);
		TEST_ASSERT(tokens[5].rowEnd == 0);
		TEST_ASSERT(tokens[5].columnEnd == 11);
		//[genius]
		TEST_ASSERT(tokens[6].start == 12);
		TEST_ASSERT(tokens[6].length == 6);
		TEST_ASSERT(tokens[6].token == 2);
		TEST_ASSERT(tokens[6].rowStart == 0);
		TEST_ASSERT(tokens[6].columnStart == 12);
		TEST_ASSERT(tokens[6].rowEnd == 0);
		TEST_ASSERT(tokens[6].columnEnd == 17);
		//[  ]
		TEST_ASSERT(tokens[7].start == 18);
		TEST_ASSERT(tokens[7].length == 2);
		TEST_ASSERT(tokens[7].token == 1);
		TEST_ASSERT(tokens[7].rowStart == 0);
		TEST_ASSERT(tokens[7].columnStart == 18);
		TEST_ASSERT(tokens[7].rowEnd == 0);
		TEST_ASSERT(tokens[7].columnEnd == 19);
		//[1234]
		TEST_ASSERT(tokens[8].start == 20);
		TEST_ASSERT(tokens[8].length == 4);
		TEST_ASSERT(tokens[8].token == 0);
		TEST_ASSERT(tokens[8].rowStart == 0);
		TEST_ASSERT(tokens[8].columnStart == 20);
		TEST_ASSERT(tokens[8].rowEnd == 0);
		TEST_ASSERT(tokens[8].columnEnd == 23);
	};

	TEST_CASE(L"Test RegexLexer 1")
	{
		List<WString> codes;
		codes.Add(L"/d+");
		codes.Add(L"/s+");
		codes.Add(L"[a-zA-Z_]/w*");
		RegexLexer lexer(codes);

		{
			List<RegexToken> tokens;
			CopyFrom(tokens, lexer.Parse(L"vczh is$$a&&genius  1234"));
			TestRegexLexer1Validation(tokens);
		}
		{
			List<RegexToken> tokens;
			lexer.Parse(L"vczh is$$a&&genius  1234").ReadToEnd(tokens);
			TestRegexLexer1Validation(tokens);
		}
	});

	auto TestRegexLexer2Validation = [](List<RegexToken>& tokens)
	{
		TEST_ASSERT(tokens.Count() == 19);
		for (vint i = 0; i < tokens.Count(); i++)
		{
			TEST_ASSERT(tokens[i].completeToken == true);
		}
		//[12345]
		TEST_ASSERT(tokens[0].start == 0);
		TEST_ASSERT(tokens[0].length == 5);
		TEST_ASSERT(tokens[0].token == 0);
		TEST_ASSERT(tokens[0].rowStart == 0);
		TEST_ASSERT(tokens[0].columnStart == 0);
		TEST_ASSERT(tokens[0].rowEnd == 0);
		TEST_ASSERT(tokens[0].columnEnd == 4);
		//[vczh]
		TEST_ASSERT(tokens[1].start == 5);
		TEST_ASSERT(tokens[1].length == 4);
		TEST_ASSERT(tokens[1].token == 1);
		TEST_ASSERT(tokens[1].rowStart == 0);
		TEST_ASSERT(tokens[1].columnStart == 5);
		TEST_ASSERT(tokens[1].rowEnd == 0);
		TEST_ASSERT(tokens[1].columnEnd == 8);
		//[ ]
		TEST_ASSERT(tokens[2].start == 9);
		TEST_ASSERT(tokens[2].length == 1);
		TEST_ASSERT(tokens[2].token == -1);
		TEST_ASSERT(tokens[2].rowStart == 0);
		TEST_ASSERT(tokens[2].columnStart == 9);
		TEST_ASSERT(tokens[2].rowEnd == 0);
		TEST_ASSERT(tokens[2].columnEnd == 9);
		//[is]
		TEST_ASSERT(tokens[3].start == 10);
		TEST_ASSERT(tokens[3].length == 2);
		TEST_ASSERT(tokens[3].token == 1);
		TEST_ASSERT(tokens[3].rowStart == 0);
		TEST_ASSERT(tokens[3].columnStart == 10);
		TEST_ASSERT(tokens[3].rowEnd == 0);
		TEST_ASSERT(tokens[3].columnEnd == 11);
		//[ ]
		TEST_ASSERT(tokens[4].start == 12);
		TEST_ASSERT(tokens[4].length == 1);
		TEST_ASSERT(tokens[4].token == -1);
		TEST_ASSERT(tokens[4].rowStart == 0);
		TEST_ASSERT(tokens[4].columnStart == 12);
		TEST_ASSERT(tokens[4].rowEnd == 0);
		TEST_ASSERT(tokens[4].columnEnd == 12);
		//[a]
		TEST_ASSERT(tokens[5].start == 13);
		TEST_ASSERT(tokens[5].length == 1);
		TEST_ASSERT(tokens[5].token == 1);
		TEST_ASSERT(tokens[5].rowStart == 0);
		TEST_ASSERT(tokens[5].columnStart == 13);
		TEST_ASSERT(tokens[5].rowEnd == 0);
		TEST_ASSERT(tokens[5].columnEnd == 13);
		//[ ]
		TEST_ASSERT(tokens[6].start == 14);
		TEST_ASSERT(tokens[6].length == 1);
		TEST_ASSERT(tokens[6].token == -1);
		TEST_ASSERT(tokens[6].rowStart == 0);
		TEST_ASSERT(tokens[6].columnStart == 14);
		TEST_ASSERT(tokens[6].rowEnd == 0);
		TEST_ASSERT(tokens[6].columnEnd == 14);
		//[genius]
		TEST_ASSERT(tokens[7].start == 15);
		TEST_ASSERT(tokens[7].length == 6);
		TEST_ASSERT(tokens[7].token == 1);
		TEST_ASSERT(tokens[7].rowStart == 0);
		TEST_ASSERT(tokens[7].columnStart == 15);
		TEST_ASSERT(tokens[7].rowEnd == 0);
		TEST_ASSERT(tokens[7].columnEnd == 20);
		//[!\r\n]
		TEST_ASSERT(tokens[8].start == 21);
		TEST_ASSERT(tokens[8].length == 3);
		TEST_ASSERT(tokens[8].token == -1);
		TEST_ASSERT(tokens[8].rowStart == 0);
		TEST_ASSERT(tokens[8].columnStart == 21);
		TEST_ASSERT(tokens[8].rowEnd == 0);
		TEST_ASSERT(tokens[8].columnEnd == 23);
		//[67890]
		TEST_ASSERT(tokens[9].start == 24);
		TEST_ASSERT(tokens[9].length == 5);
		TEST_ASSERT(tokens[9].token == 0);
		TEST_ASSERT(tokens[9].rowStart == 1);
		TEST_ASSERT(tokens[9].columnStart == 0);
		TEST_ASSERT(tokens[9].rowEnd == 1);
		TEST_ASSERT(tokens[9].columnEnd == 4);
		//["vczh"]
		TEST_ASSERT(tokens[10].start == 29);
		TEST_ASSERT(tokens[10].length == 6);
		TEST_ASSERT(tokens[10].token == 2);
		TEST_ASSERT(tokens[10].rowStart == 1);
		TEST_ASSERT(tokens[10].columnStart == 5);
		TEST_ASSERT(tokens[10].rowEnd == 1);
		TEST_ASSERT(tokens[10].columnEnd == 10);
		//["is"]
		TEST_ASSERT(tokens[11].start == 35);
		TEST_ASSERT(tokens[11].length == 4);
		TEST_ASSERT(tokens[11].token == 2);
		TEST_ASSERT(tokens[11].rowStart == 1);
		TEST_ASSERT(tokens[11].columnStart == 11);
		TEST_ASSERT(tokens[11].rowEnd == 1);
		TEST_ASSERT(tokens[11].columnEnd == 14);
		//[ ]
		TEST_ASSERT(tokens[12].start == 39);
		TEST_ASSERT(tokens[12].length == 1);
		TEST_ASSERT(tokens[12].token == -1);
		TEST_ASSERT(tokens[12].rowStart == 1);
		TEST_ASSERT(tokens[12].columnStart == 15);
		TEST_ASSERT(tokens[12].rowEnd == 1);
		TEST_ASSERT(tokens[12].columnEnd == 15);
		//["a"]
		TEST_ASSERT(tokens[13].start == 40);
		TEST_ASSERT(tokens[13].length == 3);
		TEST_ASSERT(tokens[13].token == 2);
		TEST_ASSERT(tokens[13].rowStart == 1);
		TEST_ASSERT(tokens[13].columnStart == 16);
		TEST_ASSERT(tokens[13].rowEnd == 1);
		TEST_ASSERT(tokens[13].columnEnd == 18);
		//["genius"]
		TEST_ASSERT(tokens[14].start == 43);
		TEST_ASSERT(tokens[14].length == 8);
		TEST_ASSERT(tokens[14].token == 2);
		TEST_ASSERT(tokens[14].rowStart == 1);
		TEST_ASSERT(tokens[14].columnStart == 19);
		TEST_ASSERT(tokens[14].rowEnd == 1);
		TEST_ASSERT(tokens[14].columnEnd == 26);
		//["!"]
		TEST_ASSERT(tokens[15].start == 51);
		TEST_ASSERT(tokens[15].length == 3);
		TEST_ASSERT(tokens[15].token == 2);
		TEST_ASSERT(tokens[15].rowStart == 1);
		TEST_ASSERT(tokens[15].columnStart == 27);
		TEST_ASSERT(tokens[15].rowEnd == 1);
		TEST_ASSERT(tokens[15].columnEnd == 29);
		//[\r\n]
		TEST_ASSERT(tokens[16].start == 54);
		TEST_ASSERT(tokens[16].length == 2);
		TEST_ASSERT(tokens[16].token == -1);
		TEST_ASSERT(tokens[16].rowStart == 1);
		TEST_ASSERT(tokens[16].columnStart == 30);
		TEST_ASSERT(tokens[16].rowEnd == 1);
		TEST_ASSERT(tokens[16].columnEnd == 31);
		//[hey]
		TEST_ASSERT(tokens[17].start == 56);
		TEST_ASSERT(tokens[17].length == 3);
		TEST_ASSERT(tokens[17].token == 1);
		TEST_ASSERT(tokens[17].rowStart == 2);
		TEST_ASSERT(tokens[17].columnStart == 0);
		TEST_ASSERT(tokens[17].rowEnd == 2);
		TEST_ASSERT(tokens[17].columnEnd == 2);
		//[!]
		TEST_ASSERT(tokens[18].start == 59);
		TEST_ASSERT(tokens[18].length == 1);
		TEST_ASSERT(tokens[18].token == -1);
		TEST_ASSERT(tokens[18].rowStart == 2);
		TEST_ASSERT(tokens[18].columnStart == 3);
		TEST_ASSERT(tokens[18].rowEnd == 2);
		TEST_ASSERT(tokens[18].columnEnd == 3);
	};

	TEST_CASE(L"Test RegexLexer 2")
	{
		List<WString> codes;
		codes.Add(L"/d+");
		codes.Add(L"[a-zA-Z_]/w*");
		codes.Add(L"\"[^\"]*\"");
		RegexLexer lexer(codes);

		WString input =
			L"12345vczh is a genius!"		L"\r\n"
			L"67890\"vczh\"\"is\" \"a\"\"genius\"\"!\""		L"\r\n"
			L"hey!";
		{
			List<RegexToken> tokens;
			CopyFrom(tokens, lexer.Parse(input));
			TestRegexLexer2Validation(tokens);
		}
		{
			List<RegexToken> tokens;
			lexer.Parse(input).ReadToEnd(tokens);
			TestRegexLexer2Validation(tokens);
		}
	});

	TEST_CASE(L"Test RegexLexer 2 (Serialization)")
	{
		MemoryStream lexerStream;
		{
			List<WString> codes;
			codes.Add(L"/d+");
			codes.Add(L"[a-zA-Z_]/w*");
			codes.Add(L"\"[^\"]*\"");
			RegexLexer(codes).Serialize(lexerStream);
		}
		lexerStream.SeekFromBegin(0);
		RegexLexer lexer(lexerStream);

		WString input =
			L"12345vczh is a genius!"		L"\r\n"
			L"67890\"vczh\"\"is\" \"a\"\"genius\"\"!\""		L"\r\n"
			L"hey!";
		{
			List<RegexToken> tokens;
			CopyFrom(tokens, lexer.Parse(input));
			TestRegexLexer2Validation(tokens);
		}
		{
			List<RegexToken> tokens;
			lexer.Parse(input).ReadToEnd(tokens);
			TestRegexLexer2Validation(tokens);
		}
	});

	TEST_CASE(L"Test RegexLexer 3")
	{
		{
			List<WString> codes;
			codes.Add(L"unit");
			codes.Add(L"/w+");
			RegexLexer lexer(codes);
			{
				WString input = L"unit";
				List<RegexToken> tokens;
				CopyFrom(tokens, lexer.Parse(input));
				TEST_ASSERT(tokens.Count() == 1);
				TEST_ASSERT(tokens[0].start == 0);
				TEST_ASSERT(tokens[0].length == 4);
				TEST_ASSERT(tokens[0].token == 0);
				TEST_ASSERT(tokens[0].rowStart == 0);
				TEST_ASSERT(tokens[0].columnStart == 0);
				TEST_ASSERT(tokens[0].rowEnd == 0);
				TEST_ASSERT(tokens[0].columnEnd == 3);
			}
			{
				WString input = L"vczh";
				List<RegexToken> tokens;
				CopyFrom(tokens, lexer.Parse(input));
				TEST_ASSERT(tokens.Count() == 1);
				TEST_ASSERT(tokens[0].start == 0);
				TEST_ASSERT(tokens[0].length == 4);
				TEST_ASSERT(tokens[0].token == 1);
				TEST_ASSERT(tokens[0].rowStart == 0);
				TEST_ASSERT(tokens[0].columnStart == 0);
				TEST_ASSERT(tokens[0].rowEnd == 0);
				TEST_ASSERT(tokens[0].columnEnd == 3);
			}
		}
		{
			List<WString> codes;
			codes.Add(L"/w+");
			codes.Add(L"unit");
			RegexLexer lexer(codes);
			{
				WString input = L"unit";
				List<RegexToken> tokens;
				CopyFrom(tokens, lexer.Parse(input));
				TEST_ASSERT(tokens.Count() == 1);
				TEST_ASSERT(tokens[0].start == 0);
				TEST_ASSERT(tokens[0].length == 4);
				TEST_ASSERT(tokens[0].token == 0);
				TEST_ASSERT(tokens[0].rowStart == 0);
				TEST_ASSERT(tokens[0].columnStart == 0);
				TEST_ASSERT(tokens[0].rowEnd == 0);
				TEST_ASSERT(tokens[0].columnEnd == 3);
			}
			{
				WString input = L"vczh";
				List<RegexToken> tokens;
				CopyFrom(tokens, lexer.Parse(input));
				TEST_ASSERT(tokens.Count() == 1);
				TEST_ASSERT(tokens[0].start == 0);
				TEST_ASSERT(tokens[0].length == 4);
				TEST_ASSERT(tokens[0].token == 0);
				TEST_ASSERT(tokens[0].rowStart == 0);
				TEST_ASSERT(tokens[0].columnStart == 0);
				TEST_ASSERT(tokens[0].rowEnd == 0);
				TEST_ASSERT(tokens[0].columnEnd == 3);
			}
		}
	});

	TEST_CASE(L"Test RegexLexer 4")
	{
		{
			List<WString> codes;
			codes.Add(L"=");
			codes.Add(L"==");
			RegexLexer lexer(codes);
			{
				WString input = L"=";
				List<RegexToken> tokens;
				CopyFrom(tokens, lexer.Parse(input));
				TEST_ASSERT(tokens.Count() == 1);
				TEST_ASSERT(tokens[0].start == 0);
				TEST_ASSERT(tokens[0].length == 1);
				TEST_ASSERT(tokens[0].token == 0);
				TEST_ASSERT(tokens[0].rowStart == 0);
				TEST_ASSERT(tokens[0].columnStart == 0);
				TEST_ASSERT(tokens[0].rowEnd == 0);
				TEST_ASSERT(tokens[0].columnEnd == 0);
			}
			{
				WString input = L"==";
				List<RegexToken> tokens;
				CopyFrom(tokens, lexer.Parse(input));
				TEST_ASSERT(tokens.Count() == 1);
				TEST_ASSERT(tokens[0].start == 0);
				TEST_ASSERT(tokens[0].length == 2);
				TEST_ASSERT(tokens[0].token == 1);
				TEST_ASSERT(tokens[0].rowStart == 0);
				TEST_ASSERT(tokens[0].columnStart == 0);
				TEST_ASSERT(tokens[0].rowEnd == 0);
				TEST_ASSERT(tokens[0].columnEnd == 1);
			}
		}
		{
			List<WString> codes;
			codes.Add(L"==");
			codes.Add(L"=");
			RegexLexer lexer(codes);
			{
				WString input = L"=";
				List<RegexToken> tokens;
				CopyFrom(tokens, lexer.Parse(input));
				TEST_ASSERT(tokens.Count() == 1);
				TEST_ASSERT(tokens[0].start == 0);
				TEST_ASSERT(tokens[0].length == 1);
				TEST_ASSERT(tokens[0].token == 1);
				TEST_ASSERT(tokens[0].rowStart == 0);
				TEST_ASSERT(tokens[0].columnStart == 0);
				TEST_ASSERT(tokens[0].rowEnd == 0);
				TEST_ASSERT(tokens[0].columnEnd == 0);
			}
			{
				WString input = L"==";
				List<RegexToken> tokens;
				CopyFrom(tokens, lexer.Parse(input));
				TEST_ASSERT(tokens.Count() == 1);
				TEST_ASSERT(tokens[0].start == 0);
				TEST_ASSERT(tokens[0].length == 2);
				TEST_ASSERT(tokens[0].token == 0);
				TEST_ASSERT(tokens[0].rowStart == 0);
				TEST_ASSERT(tokens[0].columnStart == 0);
				TEST_ASSERT(tokens[0].rowEnd == 0);
				TEST_ASSERT(tokens[0].columnEnd == 1);
			}
		}
	});

	auto TestRegexLexer5Validation = [](List<RegexToken>& tokens)
	{
		TEST_ASSERT(tokens.Count() == 2);
		//[123]
		TEST_ASSERT(tokens[0].start == 0);
		TEST_ASSERT(tokens[0].length == 3);
		TEST_ASSERT(tokens[0].token == 0);
		TEST_ASSERT(tokens[0].rowStart == 0);
		TEST_ASSERT(tokens[0].columnStart == 0);
		TEST_ASSERT(tokens[0].rowEnd == 0);
		TEST_ASSERT(tokens[0].columnEnd == 2);
		TEST_ASSERT(tokens[0].completeToken == true);
		//["456]
		TEST_ASSERT(tokens[1].start == 3);
		TEST_ASSERT(tokens[1].length == 4);
		TEST_ASSERT(tokens[1].token == 1);
		TEST_ASSERT(tokens[1].rowStart == 0);
		TEST_ASSERT(tokens[1].columnStart == 3);
		TEST_ASSERT(tokens[1].rowEnd == 0);
		TEST_ASSERT(tokens[1].columnEnd == 6);
		TEST_ASSERT(tokens[1].completeToken == false);
	};

	TEST_CASE(L"Test RegexLexer 5")
	{
		List<WString> codes;
		codes.Add(L"/d+");
		codes.Add(L"\"[^\"]*\"");
		RegexLexer lexer(codes);

		WString input = L"123\"456";
		{
			List<RegexToken> tokens;
			CopyFrom(tokens, lexer.Parse(input));
			TestRegexLexer5Validation(tokens);
		}
		{
			List<RegexToken> tokens;
			lexer.Parse(input).ReadToEnd(tokens);
			TestRegexLexer5Validation(tokens);
		}
	});

	TEST_CATEGORY(L"Unicode")
	{
		List<U8String> codes;
		codes.Add(u8"[𣂕𣴑𣱳𦁚]+");
		codes.Add(u8"[^𣂕𣴑𣱳𦁚]+");
		RegexLexer_<char8_t> lexer(codes);

		TEST_CASE(L"char8_t")
		{
			auto input = u8"𩰪㦲𦰗𠀼 𣂕𣴑𣱳𦁚 Vczh is genius!@我是天才";
			List<RegexToken_<char8_t>> tokens;
			CopyFrom(tokens, lexer.Parse(input));

			TEST_ASSERT(tokens.Count() == 3);

			TEST_ASSERT(tokens[0].start == 0);
			TEST_ASSERT(tokens[0].length == 16);
			TEST_ASSERT(tokens[0].token == 1);
			TEST_ASSERT(tokens[0].rowStart == 0);
			TEST_ASSERT(tokens[0].columnStart == 0);
			TEST_ASSERT(tokens[0].rowEnd == 0);
			TEST_ASSERT(tokens[0].columnEnd == 15);
			TEST_ASSERT(tokens[0].completeToken == true);

			TEST_ASSERT(tokens[1].start == 16);
			TEST_ASSERT(tokens[1].length == 16);
			TEST_ASSERT(tokens[1].token == 0);
			TEST_ASSERT(tokens[1].rowStart == 0);
			TEST_ASSERT(tokens[1].columnStart == 16);
			TEST_ASSERT(tokens[1].rowEnd == 0);
			TEST_ASSERT(tokens[1].columnEnd == 31);
			TEST_ASSERT(tokens[1].completeToken == true);

			TEST_ASSERT(tokens[2].start == 32);
			TEST_ASSERT(tokens[2].length == 29);
			TEST_ASSERT(tokens[2].token == 1);
			TEST_ASSERT(tokens[2].rowStart == 0);
			TEST_ASSERT(tokens[2].columnStart == 32);
			TEST_ASSERT(tokens[2].rowEnd == 0);
			TEST_ASSERT(tokens[2].columnEnd == 60);
			TEST_ASSERT(tokens[2].completeToken == true);
		});

		TEST_CASE(L"char16_t")
		{
			auto input = u"𩰪㦲𦰗𠀼 𣂕𣴑𣱳𦁚 Vczh is genius!@我是天才";
			List<RegexToken_<char16_t>> tokens;
			CopyFrom(tokens, lexer.Parse(input));

			TEST_ASSERT(tokens.Count() == 3);

			TEST_ASSERT(tokens[0].start == 0);
			TEST_ASSERT(tokens[0].length == 8);
			TEST_ASSERT(tokens[0].token == 1);
			TEST_ASSERT(tokens[0].rowStart == 0);
			TEST_ASSERT(tokens[0].columnStart == 0);
			TEST_ASSERT(tokens[0].rowEnd == 0);
			TEST_ASSERT(tokens[0].columnEnd == 7);
			TEST_ASSERT(tokens[0].completeToken == true);

			TEST_ASSERT(tokens[1].start == 8);
			TEST_ASSERT(tokens[1].length == 8);
			TEST_ASSERT(tokens[1].token == 0);
			TEST_ASSERT(tokens[1].rowStart == 0);
			TEST_ASSERT(tokens[1].columnStart == 8);
			TEST_ASSERT(tokens[1].rowEnd == 0);
			TEST_ASSERT(tokens[1].columnEnd == 15);
			TEST_ASSERT(tokens[1].completeToken == true);

			TEST_ASSERT(tokens[2].start == 16);
			TEST_ASSERT(tokens[2].length == 21);
			TEST_ASSERT(tokens[2].token == 1);
			TEST_ASSERT(tokens[2].rowStart == 0);
			TEST_ASSERT(tokens[2].columnStart == 16);
			TEST_ASSERT(tokens[2].rowEnd == 0);
			TEST_ASSERT(tokens[2].columnEnd == 36);
			TEST_ASSERT(tokens[2].completeToken == true);
		});

		TEST_CASE(L"char32_t")
		{
			auto input = U"𩰪㦲𦰗𠀼 𣂕𣴑𣱳𦁚 Vczh is genius!@我是天才";
			List<RegexToken_<char32_t>> tokens;
			CopyFrom(tokens, lexer.Parse(input));

			TEST_ASSERT(tokens.Count() == 3);

			TEST_ASSERT(tokens[0].start == 0);
			TEST_ASSERT(tokens[0].length == 5);
			TEST_ASSERT(tokens[0].token == 1);
			TEST_ASSERT(tokens[0].rowStart == 0);
			TEST_ASSERT(tokens[0].columnStart == 0);
			TEST_ASSERT(tokens[0].rowEnd == 0);
			TEST_ASSERT(tokens[0].columnEnd == 4);
			TEST_ASSERT(tokens[0].completeToken == true);

			TEST_ASSERT(tokens[1].start == 5);
			TEST_ASSERT(tokens[1].length == 4);
			TEST_ASSERT(tokens[1].token == 0);
			TEST_ASSERT(tokens[1].rowStart == 0);
			TEST_ASSERT(tokens[1].columnStart == 5);
			TEST_ASSERT(tokens[1].rowEnd == 0);
			TEST_ASSERT(tokens[1].columnEnd == 8);
			TEST_ASSERT(tokens[1].completeToken == true);

			TEST_ASSERT(tokens[2].start == 9);
			TEST_ASSERT(tokens[2].length == 21);
			TEST_ASSERT(tokens[2].token == 1);
			TEST_ASSERT(tokens[2].rowStart == 0);
			TEST_ASSERT(tokens[2].columnStart == 9);
			TEST_ASSERT(tokens[2].rowEnd == 0);
			TEST_ASSERT(tokens[2].columnEnd == 29);
			TEST_ASSERT(tokens[2].completeToken == true);
		});
	});

	TEST_CATEGORY(L"Unicode scalar boundaries")
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

		UnicodeLexerToken word8[] =
		{
			{ 0, 33, -1 },
			{ 33, 4, 0 },
			{ 37, 1, -1 },
			{ 38, 2, 0 },
			{ 40, 1, -1 },
			{ 41, 6, 0 },
			{ 47, 14, -1 },
		};
		UnicodeLexerToken scalar8[] =
		{
			{ 0, 28, -1 },
			{ 28, 4, 0 },
			{ 32, 29, -1 },
		};
		UnicodeLexerToken inverseWord8[] =
		{
			{ 0, 33, 0 },
			{ 33, 4, -1 },
			{ 37, 1, 0 },
			{ 38, 2, -1 },
			{ 40, 1, 0 },
			{ 41, 6, -1 },
			{ 47, 14, 0 },
		};

		UnicodeLexerToken word16[] =
		{
			{ 0, 17, -1 },
			{ 17, 4, 0 },
			{ 21, 1, -1 },
			{ 22, 2, 0 },
			{ 24, 1, -1 },
			{ 25, 6, 0 },
			{ 31, 6, -1 },
		};
		UnicodeLexerToken scalar16[] =
		{
			{ 0, 14, -1 },
			{ 14, 2, 0 },
			{ 16, 21, -1 },
		};
		UnicodeLexerToken inverseWord16[] =
		{
			{ 0, 17, 0 },
			{ 17, 4, -1 },
			{ 21, 1, 0 },
			{ 22, 2, -1 },
			{ 24, 1, 0 },
			{ 25, 6, -1 },
			{ 31, 6, 0 },
		};

		UnicodeLexerToken word32[] =
		{
			{ 0, 10, -1 },
			{ 10, 4, 0 },
			{ 14, 1, -1 },
			{ 15, 2, 0 },
			{ 17, 1, -1 },
			{ 18, 6, 0 },
			{ 24, 6, -1 },
		};
		UnicodeLexerToken scalar32[] =
		{
			{ 0, 8, -1 },
			{ 8, 1, 0 },
			{ 9, 21, -1 },
		};
		UnicodeLexerToken inverseWord32[] =
		{
			{ 0, 10, 0 },
			{ 10, 4, -1 },
			{ 14, 1, 0 },
			{ 15, 2, -1 },
			{ 17, 1, 0 },
			{ 18, 6, -1 },
			{ 24, 6, 0 },
		};

		TEST_CASE(L"All input encodings")
		{
			U8String input8 = u8"𩰪㦲𦰗𠀼 𣂕𣴑𣱳𦁚 Vczh is genius!@我是天才";
			AssertUnicodeLexer(wordLexer, input8, word8);
			AssertUnicodeLexer(scalarLexer, input8, scalar8);
			AssertUnicodeLexer(inverseWordLexer, input8, inverseWord8);

			U16String input16 = u"𩰪㦲𦰗𠀼 𣂕𣴑𣱳𦁚 Vczh is genius!@我是天才";
			AssertUnicodeLexer(wordLexer, input16, word16);
			AssertUnicodeLexer(scalarLexer, input16, scalar16);
			AssertUnicodeLexer(inverseWordLexer, input16, inverseWord16);

			U32String input32 = U"𩰪㦲𦰗𠀼 𣂕𣴑𣱳𦁚 Vczh is genius!@我是天才";
			AssertUnicodeLexer(wordLexer, input32, word32);
			AssertUnicodeLexer(scalarLexer, input32, scalar32);
			AssertUnicodeLexer(inverseWordLexer, input32, inverseWord32);

			WString input = L"𩰪㦲𦰗𠀼 𣂕𣴑𣱳𦁚 Vczh is genius!@我是天才";
#if defined VCZH_WCHAR_UTF16
			AssertUnicodeLexer(wordLexer, input, word16);
			AssertUnicodeLexer(scalarLexer, input, scalar16);
			AssertUnicodeLexer(inverseWordLexer, input, inverseWord16);
#elif defined VCZH_WCHAR_UTF32
			AssertUnicodeLexer(wordLexer, input, word32);
			AssertUnicodeLexer(scalarLexer, input, scalar32);
			AssertUnicodeLexer(inverseWordLexer, input, inverseWord32);
#endif
		});
	});
}
