In the latest commit 2978c8995512f160294525e9a213a9f26c155095 you can see some changes applied to `RegexLexerWalker_<T>`.
I believe `T` here is the actual unicode encoding character type,
so it should follow what `Regex_<T>` has done to convert the input to `char32_t` before calling `pure->` instead of using `static_cast<char32_t>`.
The current test cases pass might because such scenario is not really tested.
So you are going to create test cases for that.
In `TestRegex.cpp` you should already see some complex characters not falling in UTF-7 which should expose the issue.
The tricky part is `TestRegex.cpp` is stored in UTF-8 with BOM so that Windows won't mess up by interpreting the file with any local code page.
When you are fixing `TestLexer.cpp` and `TestColorizer.cpp` you should do the same thing (if they have not been done).
For making things easier you could define `/w+` as a token and the rest would become non-defined tokens, the walker and the colorizer should respond to that correctly.
The walker should walk only one character (I mean a real character not just a code point).
So you can use that string in `TestRegex.cpp:337` as the string to test, and test lexer and colorizer against all 4 types wchar_t, char8_t, char16_t, char32_t.
Colorizer is depending on lexer should I guess only lexer needs to fix, correct me if I am wrong.
Create proper test cases to repro the issue, and fix it.
