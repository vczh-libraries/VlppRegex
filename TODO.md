# TODO

## 2.0

- Regex supports UTF-16 (instead of UCS-2) on MSVC.
  - Always convert to UTF-32 internally.
    - using `UtfStringFrom32Reader<T>` and `UtfStringTo32Reader<T>`.
  - `Regex` and a few other classes become template class instantiations.
    - Class names for `WString` will still be unchanged, like `using Regex = RegexUnicode<wchar_t>;` or something.
  - Token will have char component position and `char32_t` component position at the same time.
    - Original field name will be kept for char component position.
  - There is no way to say a specific character in regex which does not fall in UTF-16 range (0 - 10FFFF).
  - The big char component categorizing array will be split to two parts:
    - `0 < x < 128`: an array
    - otherwise: a map using ranges as keys
- Move platform-dependent code to separated files as what `Vlpp` does.

## Optional
