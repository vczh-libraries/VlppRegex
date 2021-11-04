# TODO

## 2.0

- Update `PureInterpretor` to accept `wchar_t`, `char8_t`, `char16_t`, `char32_t` as input.
- Update `RichInterpretor` to accept `wchar_t`, `char8_t`, `char16_t`, `char32_t` as input.
- Token will have char component position and `char32_t` component position at the same time.
  - Test `PureInterpretor`
  - Test `RichInterpretor`
  - Test `Regex`
  - Test `RegexLexerWalker`
  - Test `RegexLexerColorizer`
- Ensure there is no way to say a specific character in regex which does not fall in UTF-16 range (0 - 10FFFF).
- Update all classes in `Regex.h` to template class.
  - `Regex` will become `RegexUtf<T>`.
  - `using Regex = RegexUtf<wchar_t>`.
- Merge branch `utf32` back to `master`
  - Undo all unit tests except `TestParser.cpp` and new added source files.
  - Ensure `wchar_t` version still work.
- Optimize `PureInterpretor::CharMap`, only 0 - 127 is in an error, others are in a range map.
- Move platform-dependent code to separated files as what `Vlpp` does.

## Optional
