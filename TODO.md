# TODO

## 2.0

- Ensure positions in results are in input character format.
  - Test `PureInterpretor`
  - Test `RichInterpretor`
  - Test `Regex`
  - Test `RegexLexerWalker`
  - Test `RegexLexerColorizer`
- Ensure there is no way to say a specific character in regex which does not fall in UTF-16 range (0 - 10FFFF).
- Merge branch `utf32` back to `master`
  - Undo all unit tests except `TestParser.cpp` and new added source files.
  - Ensure `wchar_t` version still work.
- Optimize `PureInterpretor::CharMap`, only 0 - 127 is in an error, others are in a range map.
- Move platform-dependent code to separated files as what `Vlpp` does.

## Optional
