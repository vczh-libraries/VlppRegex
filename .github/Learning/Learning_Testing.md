# !!!LEARNING!!!

# Orders

- Exercise VlppRegex Unicode paths with scalar-sensitive encoded-input regressions [1]

# Refinements

## Exercise VlppRegex Unicode paths with scalar-sensitive encoded-input regressions

Use one common token-definition encoding and exercise `wchar_t`, `char8_t`, `char16_t`, and `char32_t` input paths through `Parse`, both `Walk` overloads and `IsClosedToken`, `Colorize`, and `Pass`. Use `/w+` to expose unrecognized fallback that restarts inside a multi-code-unit scalar, an explicit token such as `[𦁚]+` or `[𣂕𣴑𣱳𦁚]+` for a scalar-sensitive recognized transition, and `/W+` only as positive control because raw UTF-8 bytes or UTF-16 surrogates can also satisfy it.

Assert exact token and callback counts, starts, lengths, rows, columns, complete-cluster spans, incomplete-prefix neutrality, token-boundary restart, and pending-state copy or restore in source-code-unit coordinates. Include exact-length non-null-terminated buffers, condition `wchar_t` expectations on UTF-16 versus UTF-32, prove the focused tests fail before the fix, and then run targeted and complete suites. Preserve UTF-8 BOM bytes `EF BB BF` in the existing Unicode test files.
