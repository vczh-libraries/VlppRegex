In the latest commit 2978c8995512f160294525e9a213a9f26c155095 you can see some changes applied to `RegexLexerWalker_<T>`.
`T` is the encoded code-unit type for encoded-buffer APIs.
Callers that own encoded text should follow what `Regex_<T>` has done: decode complete source clusters to `char32_t` before calling `pure->`; `static_cast<char32_t>` is not UTF decoding.
The current test cases pass might because such scenario is not really tested.
So you are going to create test cases for that.
In `TestRegex.cpp` you should already see supplementary-plane characters outside the BMP, which should expose the issue because they require multiple UTF-8 or UTF-16 code units.
The tricky part is `TestRegex.cpp` is stored in UTF-8 with BOM so that Windows won't mess up by interpreting the file with any local code page.
When you are fixing `TestLexer.cpp`, `TestWalker.cpp` and `TestColorizer.cpp` you should do the same thing (if they have not been done).
For making things easier, define `/w+` as a token so the supplementary characters become undefined input, add `/W+` as the opposite recognized-input control, and use an explicit supplementary character class for a scalar-sensitive recognized repro.
One walker step means exactly one decoded `char32_t` Unicode scalar. A caller reading encoded text must therefore advance by the complete source cluster: for example, four `char8_t` code units for a four-byte UTF-8 scalar.
Use the string in `TestRegex.cpp:337` and test the lexer, walker and colorizer against all four input types: `wchar_t`, `char8_t`, `char16_t` and `char32_t`.
The unrecognized lexer fallback, walker range operations and colorizer are independent paths and all must be fixed where necessary.
Create proper test cases to repro the issue, and fix it.

## DETAILS

- Treat the text in `TestRegex.cpp:337` as valid Unicode containing supplementary-plane scalar values. One DFA transition and one call to either `RegexLexerWalker_<T>::Walk` overload consume exactly one decoded `char32_t` scalar. Restore those overloads to `char32_t`; they do not decode `T`. Code that owns an encoded range, including `IsClosedToken` and the colorizer, must use a bounded UTF reader and advance by the complete `SourceCluster().size`. Use one token-definition encoding and exercise the input side explicitly through `Parse<TInput>`, `Walk<TInput>` and `Colorize<TInput>` for `wchar_t`, `char8_t`, `char16_t` and `char32_t`; a 16-combination token-definition/input matrix is unnecessary.
- Add the regression tests before changing the implementation and ensure that they fail against commit `2978c8995512f160294525e9a213a9f26c155095` for the affected encodings. Keep `char32_t` as the control, and account for `wchar_t` being UTF-16 on Windows and UTF-32 on Linux/macOS.
- For every input type and for each of `Parse`, `Walk`/`IsClosedToken` and `Colorize`, cover both directions using U+2605A (`𦁚`):
  - With `/w+`, the scalar is undefined and must be consumed once as a complete encoded source cluster.
  - With a repeatable explicit token such as `[𦁚]+`, the scalar is token 0 and must cause exactly one DFA transition while retaining the complete encoded range.
  - Also add the requested `/W+` recognized-input control. `/W` is implemented, but this control does not replace the explicit supplementary token because raw UTF-8 bytes and UTF-16 surrogates also satisfy `/W` in the broken implementation.
- In `TestLexer.cpp`, use `/w+` to verify that matching never restarts inside an unrecognized scalar's encoding. U+2605A is UTF-8 `F0 A6 81 9A`; the current one-code-unit fallback can restart at `81 9A` and incorrectly recognize those bytes as `Z`. The expected UTF-8 tokenization begins with one undefined token `(start=0, length=33, token=-1)`, followed by the actual ASCII word tokens; no recognized token may point into a multibyte scalar. Add the explicit `[𦁚]+` recognized-token counterpart and assert token starts, lengths, `rowStart`, `columnStart`, `rowEnd` and `columnEnd` in source-code-unit coordinates.
- Add direct walker coverage in `TestWalker.cpp`, which is the existing test file for `RegexLexerWalker_<T>`:
  - Walk one decoded supplementary scalar against both `/w+` and `[𦁚]+`, and assert the resulting state, token, `finalState`, `previousTokenStop` and `GetRelatedToken`.
  - Exercise both `Walk` overloads and the restart-on-token-boundary path.
  - For both `IsClosedToken` overloads, verify that encoded U+2605A is closed/unrecognized with `/w+`. Then use the repeatable definition `[𦁚]+` to verify that one encoded scalar is not closed and that appending a nonmatching scalar closes it. Do not use an exact one-character literal for the latter assertion because its final DFA state is already dead.
- In `TestColorizer.cpp`, keep the focused supplementary token `[𣂕𣴑𣱳𦁚]+` in addition to the `/w+` undefined-input case and the `/W+` control. Assert both the per-code-unit colors and the actual callback records `(start, length, token)`. For a single U+2605A, the undefined and explicit-token cases must each produce one callback spanning 4 `char8_t`, 2 `char16_t` or 1 `char32_t` code units, with token `-1` or 0 respectively. A flat color array cannot distinguish one callback for a decoded scalar from separate callbacks for each UTF-8 byte or UTF-16 surrogate.
- Keep all public starts, lengths, rows, columns and callback ranges in units of the original `TInput` buffer. Decoding is only for DFA transitions; all code units belonging to the same scalar must remain in the same logical token/color callback, and every bounded-buffer path must make forward progress without reading past `length`.
- Add exact-length, non-null-terminated input buffers ending immediately after a complete supplementary scalar for `Colorize(const T*, vint)` and `IsClosedToken(const T*, vint)`. These cases must prove that the implementation uses the supplied bound rather than reading an available terminator.
- Subject to the open `Pass` review comment below, test a supplementary scalar through `RegexLexerColorizer_<T>::Pass` for every `T`, inspect `GetInternalState().currentState`, and continue the token with the next `Colorize` call. Existing ASCII CR/LF `Pass` coverage cannot expose this bug.
- Include every independent raw-code-unit path in the implementation audit:
  - `RegexTokenEnumerator<T>::Next` currently forces `result.length = 1` for an unrecognized input.
  - Both `RegexLexerWalker_<T>::Walk` overloads should pass their `char32_t` argument directly to `PureInterpretor::Transit`; `IsClosedToken` must replace its raw-code-unit loop with bounded decoding.
  - `RegexLexerColorizer_<T>::WalkOneToken` must decode its bounded `T` range and map scalar/token boundaries back to source-code-unit offsets.
  - `RegexLexerColorizer_<T>::Pass(T)` has the same single-code-unit contract problem and needs the explicit contract requested below.
  Reuse the existing UTF range-reader/conversion facilities so decoding stays linear and source-cluster offsets are retained.
- Update the public comments and examples in `Regex.h` for the `Walk` and `Pass` signature/semantic changes. Define a Unicode scalar, encoded code unit and grapheme cluster accurately. Valid, complete UTF input may remain a precondition, but bounded overloads must not read beyond their buffers.
- `TestRegex.cpp`, `TestLexer.cpp`, `TestWalker.cpp`, `TestColorizer.cpp` and `ColorizerCommon.h` already begin with the UTF-8 BOM bytes `EF BB BF`. Preserve those bytes while editing; no BOM conversion is needed.

## VERIFICATION

1. Add focused tests first and run `TestLexer.cpp`, `TestWalker.cpp` and `TestColorizer.cpp` against the current implementation. Confirm that the `/w+` lexer test exposes the false UTF-8 word token, that the explicit supplementary-token walker/colorizer cases fail for `char8_t` and `char16_t` (and Windows `wchar_t`), and that the `char32_t` control passes. `/W+` is additional control coverage, not the only red repro.
2. After the fix, verify both the undefined `/w+` and recognized `[𦁚]+` cases through `Parse`, `Walk`/`IsClosedToken` and `Colorize` for all four input types. Starts, lengths, rows and columns must be source-code-unit counts: `wchar_t` expectations follow the UTF-16 row on Windows and the UTF-32 row elsewhere.
3. For colorization, verify that callback ranges are ordered, non-overlapping, make forward progress and cover exactly the supplied input buffer. A single U+2605A callback has length 4 for `char8_t`, 2 for `char16_t`, 1 for `char32_t`, and the corresponding `wchar_t` length. The supplementary token `[𣂕𣴑𣱳𦁚]+` should occupy `[16,32)` for `char8_t`, `[8,16)` for `char16_t`, `[5,9)` for `char32_t`, and the platform-dependent `wchar_t` range. Assert `Colorize(...) == nullptr` when there is no `extendProc`; inspect `GetInternalState().currentState` separately when verifying DFA state.
4. Verify both successful transitions and token-boundary restarts in the walker. Test both `IsClosedToken` overloads with `[𦁚]+`, including exact-length buffers without a trailing NUL. Confirm that no UTF-8 continuation byte or UTF-16 surrogate is independently passed to `PureInterpretor::Transit`.
5. After settling the `Pass` contract, pass a supplementary scalar for all four `RegexLexerColorizer_<T>` instantiations, assert the intermediate DFA state, and continue the token in a subsequent `Colorize` call. If `extendProc` is installed, also verify that it receives the complete temporary `T` source cluster and its code-unit length.
6. Build with the repository-provided wrapper, run the three targeted test files with `/C`, and then run the complete `UnitTest` suite as required by `Project.md`. On Windows, ensure none of the target files are marked `[SKIPPED]`, inspect the final execution log for the complete pass summary and confirm that no memory-leak dump follows it. On Linux/macOS, use `.github/Ubuntu/build.sh` from `Test/Linux` and run `Bin/UnitTest` asynchronously from that directory.
7. Recheck that the edited Unicode test files still begin with `EF BB BF`.

## REVIEW COMMENTS

### DEFINE THE WALKER INPUT UNIT AND `PASS` CONTRACT

**review comment**: `PureInterpretor::Transit` consumes a decoded `char32_t`, but `RegexLexerWalker_<T>::Walk(T, ...)` receives only one encoding code unit. A single `char8_t` byte or UTF-16 surrogate cannot be statelessly converted into a supplementary scalar, and hiding decoder state inside the shared `const` walker would break its caller-owned-state model. “One real character, not just a code point” is also ambiguous: a code point/scalar is the DFA unit, while a grapheme cluster can contain multiple scalars and is outside the current regex design. I recommend restoring the pre-templatization contract in which `Walk` consumes one `char32_t` Unicode scalar, while `T` remains the encoding type for range-based APIs such as `IsClosedToken`. Apply the same scalar-versus-range decision to `RegexLexerColorizer_<T>::Pass`, including how `RegexProc_<T>` callbacks receive the encoded text.

"one real character" means a char32_t character. So if 4 char8_t forms 1 char32_t, and when walker decides to step forward by one character, it should step forward by 4 char8_t code points. And `pure` works on char32_t so it perfectly fits the implementation. In the original problem description, using `/w+` as a token means all non-UTF7 characters are illegal tokens, which would cause walker to step forward one character, that's what I meant. If non-UTF7 characters become valid tokens, `pure` will decide how many actual char32_t characters need to step forward. So yeah you can also create an opposite test case so that `/W+` becomes a token. I can't remember if `/W` has already implemented but if not you can use `[^..]` instead. By doing this it covers both way.

### USE DIFFERENT RED REPROS FOR LEXER AND COLORIZER

**review comment**: `/w+` is useful for the lexer fallback bug: after `RegexTokenEnumerator<T>::Next` advances only one byte into U+2605A, the existing UTF-8 decoder can reinterpret continuation bytes `81 9A` as ASCII `Z`. However, `/w` itself is only `[a-zA-Z0-9_]`; the current walker/colorizer also classifies each raw non-ASCII byte or surrogate as undefined, so a flat array of `-1` colors can pass before and after the fix. Keep the `/w+` lexer repro, but use a token that actually matches a supplementary scalar for walker/colorizer tests, or assert callback count and cluster-sized ranges in addition to per-unit colors.

### COLORIZER AND UNRECOGNIZED LEXER ADVANCEMENT ARE BOTH IN FIX SCOPE [CLOSED]

**review comment**: Fixing only the casts inside `RegexLexerWalker_<T>` is insufficient. Normal `RegexLexerBase_::Parse` matching decodes through `PureInterpretor::MatchHead`, but its unrecognized-input fallback still advances by one code unit. Separately, `RegexLexerColorizer_<T>::WalkOneToken` iterates raw `T` units before calling the walker, so it must decode scalars and map DFA boundaries back to source-code-unit offsets itself. `IsClosedToken` also needs a bounded decoder. Please treat these as separate root paths, guarantee forward progress, and include `TestWalker.cpp` explicitly rather than placing all walker coverage in `TestLexer.cpp`.

Agreed. Test cases should be able to catch that anyway.

### PRESERVE SOURCE OFFSETS AND CLARIFY THE FOUR-TYPE MATRIX [CLOSED]

**review comment**: `RegexLexer_<TDefinition>` uses `TDefinition` for the token definitions, while `Parse<TInput>`, `Walk<TInput>` and `Colorize<TInput>` select the input encoding. The task should test the latter four types from one common lexer definition. Token/callback positions must continue indexing the original encoded buffer, not the decoded scalar array. In particular, derive or condition `wchar_t` expectations because it is UTF-16 on Windows and UTF-32 elsewhere. The cited “not falling in UTF-7” wording should be “outside the BMP / requiring multiple UTF-8 or UTF-16 code units.”

Agreed.

### DEFINE THE `PASS` SCALAR/CALLBACK CONTRACT

**review comment**: The reply makes both `Walk` overloads unambiguously scalar APIs, and the historical API confirms `Walk(char32_t)` was the pre-templatization contract. It does not say how `RegexLexerColorizer_<T>::Pass` supplies one supplementary scalar to `RegexProc_<T>::extendProc`, whose text argument is encoded as `T`. I recommend `Pass(char32_t)`: use the existing generic UTF-32-to-`T` conversion machinery (including the direct `char32_t` path) to form a temporary encoded source cluster, call `WalkOneToken` over the whole cluster, and keep callback lengths in `T` code units. Add supplementary `Pass` and `extendProc` coverage for all four `T` instantiations.

Just keep the current semantic. By applying this change, a `RegexLexerWalker_<char32_t>` should not be changed semantically.

### KEEP THE RECOGNIZED CASE SENSITIVE TO SCALAR BOUNDARIES

**review comment**: `/W` is implemented, so the requested `/W+` case is useful positive coverage. It is not a red repro by itself for encoded-range consumers: every raw UTF-8 byte and UTF-16 surrogate is also outside `[a-zA-Z0-9_]`, allowing the broken per-code-unit implementation to produce the same `/W+` token and callback range. Keep `/W+` as a control, but retain a scalar-sensitive recognized case such as `[𦁚]+` or the focused `[𣂕𣴑𣱳𦁚]+` token and assert exact callback boundaries.

agreed.
