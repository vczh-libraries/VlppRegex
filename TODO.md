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
    - It will allow capturing (but not others) and remove capturing to make it DFA-eligible
    - When a prefix is found, run the regex with capturing.
    - The above step will be skipped if no capturing definition is found in the regex.
  - `RegexReplacer` gives a postfix
    - The postfix will be scanned to ensure all capturings are defined and no capturing is unused.
    - If there is no capturing in prefix, it will be a plain text, but escaping still in use.
  - Search all the way to find the first postfix
  - Make it a whole token, using the ID of the prefix token
    - Since the postfix is not used in tokenizing it will not occupy an ID.

## Optional
