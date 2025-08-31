# TODO

## 2.0

- Ensure there is no way to say a specific character in regex which does not fall in UTF-16 range (0 - 10FFFF).
- Optimize `PureInterpretor::CharMap`, only 0 - 127 is in an array, others are in a range map.
- Put `Regex` and `RegexLexer` to different cpp files.
- A `RegexReplacer`, which could access anonymius or named captures:
  - Use them to build a nee string
  - Join list with delimiter
  - Take anonymous and named captures as source and pairwide them to be a new list
- Enhance `RegexTokenizer`. It could handle C++ multiline string literal by:
  - Define the prefix as a token
  - `RegexReplacer` gives a postfix
  - Search all the way to find the first postfix
  - Make it a whole token, using the ID of the prefix token

## Optional
