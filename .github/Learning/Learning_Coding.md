# !!!LEARNING!!!

# Orders

- Preserve VlppRegex code-unit APIs while decoding complete Unicode scalars [1]

# Refinements

## Preserve VlppRegex code-unit APIs while decoding complete Unicode scalars

`T` in encoded-buffer APIs is the encoded code-unit type. Keep both `RegexLexerWalker_<T>::Walk(T, ...)` overloads and `RegexLexerColorizer_<T>::Pass(T)` accepting one `T` code unit per call; do not change them to `char32_t` merely because `PureInterpretor::Transit` consumes a scalar. Buffer incomplete UTF-8 or UTF-16 sequences in walker or colorizer state and call the DFA only after a complete `char32_t` scalar is available. Incomplete prefix calls are neutral, copied walker or colorizer internal state must retain pending units, and the `char32_t` path remains a direct one-unit path.

For encoded ranges, the `RegexTokenEnumerator<T>` fallback, `RegexLexerWalker_<T>::IsClosedToken`, and `RegexLexerColorizer_<T>` must decode bounded complete source clusters. Advance unrecognized input by `SourceCluster().size`, preserve public starts, lengths, rows, columns, and callback spans in original `T` code units, and never read beyond the supplied length. Valid, complete UTF input may remain a precondition.
