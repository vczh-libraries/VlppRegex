/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
Regex::Regular Expression

Classes:
	RegexString						: String Fragment
	RegexMatch						: Match Result
	Regex							: Regular Expression
	RegexToken						: Token
	RegexTokens						: Token Stream
	RegexLexer						: Tokenizer
***********************************************************************/

#ifndef VCZH_REGEX_REGEX
#define VCZH_REGEX_REGEX

#include <Vlpp.h>

namespace vl
{
	namespace regex_internal
	{
		class PureResult;
		class PureInterpretor;
		class RichResult;
		class RichInterpretor;
	}

	namespace regex
	{

/***********************************************************************
Data Structure
***********************************************************************/

		/// <summary>A sub string of the string that a <see cref="Regex"/> is matched against.</summary>
		class RegexString : public Object
		{
		protected:
			WString										value;
			vint										start;
			vint										length;

		public:
			RegexString(vint _start=0);
			RegexString(const WString& _string, vint _start, vint _length);

			/// <summary>The position of the input string in characters.</summary>
			/// <returns>The position.</returns>
			vint										Start()const;
			/// <summary>The size of the sub string in characters.</summary>
			/// <returns>The size.</returns>
			vint										Length()const;
			/// <summary>Get the sub string as a <see cref="WString"/>.</summary>
			/// <returns>The sub string.</returns>
			const WString&								Value()const;
			bool										operator==(const RegexString& string)const;
		};

		/// <summary>A match produces by a <see cref="Regex"/>.</summary>
		class RegexMatch : public Object, private NotCopyable
		{
			friend class Regex;
		public:
			typedef Ptr<RegexMatch>										Ref;
			typedef collections::List<Ref>								List;
			typedef collections::List<RegexString>						CaptureList;
			typedef collections::Group<WString, RegexString>			CaptureGroup;
		protected:
			collections::List<RegexString>				captures;
			collections::Group<WString, RegexString>	groups;
			bool										success;
			RegexString									result;

			RegexMatch(const WString& _string, regex_internal::PureResult* _result);
			RegexMatch(const WString& _string, regex_internal::RichResult* _result, regex_internal::RichInterpretor* _rich);
			RegexMatch(const RegexString& _result);
		public:
			
			/// <summary>
			/// Test if this match is a succeeded match or a failed match.
			/// A failed match will only appear when calling [M:vl.regex.Regex.Split] or [M:vl.regex.Regex.Cut].
			/// In other cases, failed matches are either not included in the result.
			/// </summary>
			/// <returns>Returns true if this match is a succeeded match.</returns>
			bool										Success()const;
			/// <summary>Get the matched sub string.</summary>
			/// <returns>The matched sub string.</returns>
			const RegexString&							Result()const;
			/// <summary>Get all sub strings that are captured anonymously.</summary>
			/// <returns>All sub strings that are captured anonymously.</returns>
			const CaptureList&							Captures()const;
			/// <summary>Get all sub strings that are captured by named groups.</summary>
			/// <returns>All sub strings that are captured by named groups.</returns>
			const CaptureGroup&							Groups()const;
		};

/***********************************************************************
Regex
***********************************************************************/

		/// <summary>
		/// <p>
		///     Regular Expression. Here is a brief description of the regular expression grammar.
		/// </p>
		/// <p>
		///     <ul>
		///         <li>
		///             <b>Charset</b>:
		///             <ul>
		///                 <li><b>a</b>, <b>[a-z]</b>, <b>[^a-z]</b></li>
		///             </ul>
		///         </li>
		///         <li>
		///             <b>Functional characters</b>:
		///             <ul>
		///                 <li><b>^</b>: the beginning of the input (DFA incompatible)</li>
		///                 <li><b>$</b>: the end of the input (DFA incompatible)</li>
		///                 <li><b>regex1|regex2</b>: match either regex1 or regex2</li>
		///             </ul>
		///         </li>
		///         <li>
		///             <b>Escaping</b> (both \ and / mean the next character is escaped):
		///             <ul>
		///                 <li>
		///                     Escaped characters:
		///                     <ul>
		///                         <li><b>\r</b>: the CR character</li>
		///                         <li><b>\n</b>: the LF character</li>
		///                         <li><b>\t</b>: the tab character</li>
		///                         <li><b>\s</b>: spacing characters (including space, \r, \n, \t)</li>
		///                         <li><b>\S</b>: non-spacing characters</li>
		///                         <li><b>\d</b>: [0-9]</li>
		///                         <li><b>\D</b>: [^0-9]</li>
		///                         <li><b>\l</b>: [a-zA-Z]</li>
		///                         <li><b>\L</b>: [^a-zA-Z]</li>
		///                         <li><b>\w</b>: [a-zA-Z0-9_]</li>
		///                         <li><b>\W</b>: [^a-zA-Z0-9_]</li>
		///                         <li><b>\.</b>: any character (this is the main different from other regex, which treat "." as any characters and "\." as the dot character)</li>
		///                         <li><b>\\</b>, <b>\/</b>, <b>\(</b>, <b>\)</b>, <b>\+</b>, <b>\*</b>, <b>\?</b>, <b>\{</b>, <b>\}</b>, <b>\[</b>, <b>\]</b>, <b>\&lt;</b>, <b>\&gt;</b>, <b>\^</b>, <b>\$</b>, <b>\!</b>, <b>\=</b>: represents itself</li>
		///                     </ul>
		///                 </li>
		///                 <li>
		///                     Escaped characters in charset defined in a square bracket:
		///                     <ul>
		///                         <li><b>\r</b>: the CR character</li>
		///                         <li><b>\n</b>: the LF character</li>
		///                         <li><b>\t</b>: the tab character</li>
		///                         <li><b>\-</b>, <b>\[</b>, <b>\]</b>, <b>\\</b>, <b>\/</b>, <b>\^</b>, <b>\$</b>: represents itself</li>
		///                     </ul>
		///                 </li>
		///             </ul>
		///         </li>
		///         <li>
		///             <b>Loops</b>:
		///             <ul>
		///                 <li><b>regex{3}</b>: repeats 3 times</li>
		///                 <li><b>regex{3,}</b>: repeats 3 or more times</li>
		///                 <li><b>regex{1,3}</b>: repeats 1 to 3 times</li>
		///                 <li><b>regex?</b>: repeats 0 or 1 times</li>
		///                 <li><b>regex*</b>: repeats 0 or more times</li>
		///                 <li><b>regex+</b>: repeats 1 or more times</li>
		///             </ul>
		///             if you add an additional <b>?</b> right after a loop, it means repeating as less as possible <b>(DFA incompatible)</b>
		///         </li>
		///         <li>
		///             <b>Capturing</b>: <b>(DFA incompatible)</b>
		///             <ul>
		///                 <li><b>(regex)</b>: No capturing, just change the operators' association</li>
		///                 <li><b>(?regex)</b>: Capture matched fragment</li>
		///                 <li><b>(&lt;name&gt;regex)</b>: Capture matched fragment in a named group called "name"</li>
		///                 <li><b>(&lt;$i&gt;)</b>: Match the i-th captured fragment, begins from 0</li>
		///                 <li><b>(&lt;$name;i&gt;)</b>: Match the i-th captured fragment in the named group called "name", begins from 0</li>
		///                 <li><b>(&lt;$name&gt;)</b>: Match any captured fragment in the named group called "name"</li>
		///             </ul>
		///         </li>
		///         <li>
		///             <b>MISC</b>
		///             <ul>
		///                 <li><b>(=regex)</b>: The prefix of the following text should match the regex, but it is not counted in the whole match <b>(DFA incompatible)</b></li>
		///                 <li><b>(!regex)</b>: Any prefix of the following text should not match the regex, and it is not counted in the whole match <b>(DFA incompatible)</b></li>
		///                 <li><b>(&lt;#name&gt;regex)</b>: Name the regex "name", and it applies here</li>
		///                 <li><b>(&lt;&name&gt;)</b>: Copy the named regex "name" here and apply</li>
		///             </ul>
		///         </li>
		///     <ul>
		/// </p>
		/// <p>
		///     The regular expression has pupre mode and rich mode.
		///     Pure mode means the regular expression is driven by a DFA, while the rich mode is not.
		/// </p>
		/// <p>
		///     The regular expression can test a string instead of matching.
		///     Testing only returns a bool very indicating success or failure.
		/// </p>
		/// </summary>
		class Regex : public Object, private NotCopyable
		{
		protected:
			regex_internal::PureInterpretor*			pure = nullptr;
			regex_internal::RichInterpretor*			rich = nullptr;

			void										Process(const WString& text, bool keepEmpty, bool keepSuccess, bool keepFail, RegexMatch::List& matches)const;
		public:
			/// <summary>Create a regular expression. It will crash if the regular expression produces syntax error.</summary>
			/// <param name="code">The regular expression in a string.</param>
			/// <param name="preferPure">Set to true to use DFA if possible.</param>
			Regex(const WString& code, bool preferPure = true);
			~Regex();

			/// <summary>Test is a DFA used to match a string.</summary>
			/// <returns>Returns true if a DFA is used.</returns>
			bool										IsPureMatch()const;
			/// <summary>Test is a DFA used to test a string. It ignores all capturing.</summary>
			/// <returns>Returns true if a DFA is used.</returns>
			bool										IsPureTest()const;

			/// <summary>Match a prefix of the text.</summary>
			/// <returns>Returns the match. Returns null if failed.</returns>
			/// <param name="text">The text to match.</param>
			RegexMatch::Ref								MatchHead(const WString& text)const;
			/// <summary>Match a sub string of the text.</summary>
			/// <returns>Returns the match. Returns null if failed.</returns>
			/// <param name="text">The text to match.</param>
			RegexMatch::Ref								Match(const WString& text)const;
			/// <summary>Match a prefix of the text, ignoring all capturing.</summary>
			/// <returns>Returns true if it succeeded.</returns>
			/// <param name="text">The text to match.</param>
			bool										TestHead(const WString& text)const;
			/// <summary>Match a sub string of the text, ignoring all capturing.</summary>
			/// <returns>Returns true if succeeded.</returns>
			/// <param name="text">The text to match.</param>
			bool										Test(const WString& text)const;
			/// <summary>Find all matched fragments in the given text, returning all matched sub strings.</summary>
			/// <param name="text">The text to match.</param>
			/// <param name="matches">Returns all succeeded matches.</param>
			void										Search(const WString& text, RegexMatch::List& matches)const;
			/// <summary>Split the text by matched sub strings, returning all unmatched sub strings.</summary>
			/// <param name="text">The text to match.</param>
			/// <param name="keepEmptyMatch">Set to true to keep all empty unmatched sub strings. This could happen when there is nothing between two matched sub strings.</param>
			/// <param name="matches">Returns all failed matches.</param>
			void										Split(const WString& text, bool keepEmptyMatch, RegexMatch::List& matches)const;
			/// <summary>Cut the text by matched sub strings, returning all matched and unmatched sub strings.</summary>
			/// <param name="text">The text to match.</param>
			/// <param name="keepEmptyMatch">Set to true to keep all empty matches. This could happen when there is nothing between two matched sub strings.</param>
			/// <param name="matches">Returns all succeeded and failed matches.</param>
			void										Cut(const WString& text, bool keepEmptyMatch, RegexMatch::List& matches)const;
		};

/***********************************************************************
Tokenizer
***********************************************************************/

		/// <summary>A token.</summary>
		struct RegexToken
		{
			/// <summary>Position in the input string in characters.</summary>
			vint										start;
			/// <summary>Size of this token in characters.</summary>
			vint										length;
			/// <summary>The token id, begins at 0, represents the regular expression in the list that matches this token. -1 means this token is produced by an error.</summary>
			vint										token;
			/// <summary>The pointer to where this token starts in the input string .</summary>
			/// <remarks>This pointer comes from a <see cref="WString"/> that used to be analyzed. You should keep a variable to that string alive, so that to keep this pointer alive.</remarks>
			const wchar_t*								reading;
			/// <summary>The "codeIndex" argument from [M:vl.regex.RegexLexer.Parse].</summary>
			vint										codeIndex;
			/// <summary>True if this token is complete. False if this token does not end here. This could happend when colorizing a text line by line.</summary>
			bool										completeToken;

			/// <summary>Row number of the first character, begins at 0.</summary>
			vint										rowStart;
			/// <summary>Column number of the first character, begins at 0.</summary>
			vint										columnStart;
			/// <summary>Row number of the last character, begins at 0.</summary>
			vint										rowEnd;
			/// <summary>Column number of the last character, begins at 0.</summary>
			vint										columnEnd;

			bool										operator==(const RegexToken& _token)const;
			bool										operator==(const wchar_t* _token)const;
		};

		/// <summary>Token information for <see cref="RegexProc::extendProc"/>.</summary>
		struct RegexProcessingToken
		{
			/// <summary>
			/// The read only start position of the token.
			/// This value will be -1 if <see cref="interTokenState"/> is not null.
			/// </summary>
			const vint									start;
			/// <summary>
			/// The length of the token, could be modified after the callback.
			/// When the callback returns, the length is not allowed to be decreased.
			/// This value will be -1 if <see cref="interTokenState"/> is not null.
			/// </summary>
			vint										length;
			/// <summary>
			/// The id of the token, could be modified after the callback.
			/// </summary>
			vint										token;
			/// <summary>
			/// The flag indicating if this token is completed, could be modified after the callback.
			/// </summary>
			bool										completeToken;
			/// <summary>
			/// The internal token state object, could be modified after the callback.
			/// When the callback returns:
			/// <ul>
			///   <li>if the completeText parameter is true in <see cref="RegexProc::extendProc"/>, it should be nullptr.</li>
			///   <li>if the token does not end at the end of the input, it should not be nullptr.</li>
			///   <li>if a token is completed in one attemp of extending, it should be nullptr.</li>
			/// </ul>
			/// </summary>
			void*										interTokenState;

			RegexProcessingToken(vint _start, vint _length, vint _token, bool _completeToken, void* _interTokenState)
				:start(_start)
				, length(_length)
				, token(_token)
				, completeToken(_completeToken)
				, interTokenState(_interTokenState)
			{
			}
		};

		using RegexInterTokenStateDeleter = void(*)(void* interTokenState);
		using RegexTokenExtendProc = void(*)(void* argument, const wchar_t* reading, vint length, bool completeText, RegexProcessingToken& processingToken);
		using RegexTokenColorizeProc =  void(*)(void* argument, vint start, vint length, vint token);

		/// <summary>Callback procedures</summary>
		struct RegexProc
		{
			/// <summary>
			/// The deleter which deletes inter token state objects created by <see cref="extendProc"/>. This callback is not called automatically.
			/// </summary>
			RegexInterTokenStateDeleter					deleter = nullptr;
			/// <summary>
			/// The token extend callback. It is called after recognizing any token, and run a customized procedure to modify the token based on the given context.
			/// If the length parameter is -1, it means the caller does not measure the incoming text buffer, which automatically indicates that the buffer is null-terminated.
			/// If the length parameter is not -1, it means the number of available characters in the buffer.
			/// The completeText parameter could be true or false. When it is false, it means that the buffer does not contain all the text.
			/// </summary>
			RegexTokenExtendProc						extendProc = nullptr;
			/// <summary>
			/// The colorizer callback. It is called when a token is recognized.
			/// </summary>
			RegexTokenColorizeProc						colorizeProc = nullptr;
			/// <summary>
			/// The argument object that is the first argument for <see cref="extendProc"/> and <see cref="colorizeProc"/>.
			/// </summary>
			void*										argument = nullptr;
		};

		/// <summary>Token collection representing the result from the lexical analyzer.</summary>
		class RegexTokens : public Object, public collections::IEnumerable<RegexToken>
		{
			friend class RegexLexer;
		protected:
			regex_internal::PureInterpretor*			pure;
			const collections::Array<vint>&				stateTokens;
			WString										code;
			vint										codeIndex;
			RegexProc									proc;
			
			RegexTokens(regex_internal::PureInterpretor* _pure, const collections::Array<vint>& _stateTokens, const WString& _code, vint _codeIndex, RegexProc _proc);
		public:
			RegexTokens(const RegexTokens& tokens);
			~RegexTokens();

			collections::IEnumerator<RegexToken>*		CreateEnumerator()const;

			/// <summary>Copy all tokens.</summary>
			/// <param name="tokens">Returns all tokens.</param>
			/// <param name="discard">A callback to decide which kind of tokens to discard. The input is [F:vl.regex.RegexToken.token]. Returns true to discard this kind of tokens.</param>
			void										ReadToEnd(collections::List<RegexToken>& tokens, bool(*discard)(vint)=0)const;
		};
		
		/// <summary>Lexical walker.</summary>
		class RegexLexerWalker : public Object
		{
			friend class RegexLexer;
		protected:
			regex_internal::PureInterpretor*			pure;
			const collections::Array<vint>&				stateTokens;
			
			RegexLexerWalker(regex_internal::PureInterpretor* _pure, const collections::Array<vint>& _stateTokens);
		public:
			RegexLexerWalker(const RegexLexerWalker& tokens);
			~RegexLexerWalker();
			
			/// <summary>Get the start DFA state number, which represents the correct state before parsing any input.</summary>
			/// <returns>The DFA state number.</returns>
			vint										GetStartState()const;
			/// <summary>Test if this state can only lead to the end of one kind of token.</summary>
			/// <returns>Returns the token index if this state can only lead to the end of one kind of token. Returns -1 if not.</returns>
			/// <param name="state">The DFA state number.</param>
			vint										GetRelatedToken(vint state)const;
			/// <summary>Step forward by one character.</summary>
			/// <param name="input">The input character.</param>
			/// <param name="state">The current state. Returns the new current state when this function returns.</param>
			/// <param name="token">Returns the token index at the end of the token.</param>
			/// <param name="finalState">Returns true if it reach the end of the token.</param>
			/// <param name="previousTokenStop">Returns true if the last character is the end of the token.</param>
			void										Walk(wchar_t input, vint& state, vint& token, bool& finalState, bool& previousTokenStop)const;
			/// <summary>Step forward by one character.</summary>
			/// <returns>Returns the new current state.</returns>
			/// <param name="input">The input character.</param>
			/// <param name="state">The current state.</param>
			vint										Walk(wchar_t input, vint state)const;
			/// <summary>Test if the input text is a complete token.</summary>
			/// <returns>Returns true if the input text is a complete token.</returns>
			/// <param name="input">The input text.</param>
			/// <param name="length">Size of the input text in characters.</param>
			bool										IsClosedToken(const wchar_t* input, vint length)const;
			/// <summary>Test if the input is a complete token.</summary>
			/// <returns>Returns true if the input text is a complete token.</returns>
			/// <param name="input">The input text.</param>
			bool										IsClosedToken(const WString& input)const;
		};

		/// <summary>Lexical colorizer.</summary>
		class RegexLexerColorizer : public Object
		{
			friend class RegexLexer;
		public:
			struct InternalState
			{
				vint									currentState = -1;
				vint									interTokenId = -1;
				void*									interTokenState = nullptr;
			};

		protected:
			RegexLexerWalker							walker;
			RegexProc									proc;
			InternalState								internalState;

			void										CallExtendProcAndColorizeProc(const wchar_t* input, vint length, RegexProcessingToken& token, bool colorize);
			vint										WalkOneToken(const wchar_t* input, vint length, vint start, bool colorize);

			RegexLexerColorizer(const RegexLexerWalker& _walker, RegexProc _proc);
		public:
			RegexLexerColorizer(const RegexLexerColorizer& colorizer);
			~RegexLexerColorizer();

			/// <summary>Get the internal state.</summary>
			/// <returns>The internal state.</returns>
			InternalState								GetInternalState();
			/// <summary>Restore the colorizer to a internal state.</summary>
			/// <param name="state">The internal state.</param>
			void										SetInternalState(InternalState state);
			/// <summary>Step forward by one character.</summary>
			/// <param name="input">The input character.</param>
			void										Pass(wchar_t input);
			/// <summary>Get the start DFA state number, which represents the correct state before colorizing any characters.</summary>
			/// <returns>The DFA state number.</returns>
			vint										GetStartState()const;
			/// <summary>Colorize a text.</summary>	GetCurrentState()const;
			/// <returns>An inter token state at the end of this line. It could be the same object which is returned from the previous call.</returns>
			/// <param name="input">The text to colorize.</param>
			/// <param name="length">Size of the text in characters.</param>
			void*										Colorize(const wchar_t* input, vint length);
		};

		/// <summary>Lexical analyzer.</summary>
		class RegexLexer : public Object, private NotCopyable
		{
		protected:
			regex_internal::PureInterpretor*			pure = nullptr;
			collections::Array<vint>					ids;
			collections::Array<vint>					stateTokens;
			RegexProc									proc;

		public:
			/// <summary>Create a lexical analyzer by a set of regular expressions. [F:vl.regex.RegexToken.token] will be the index of the matched regular expression.</summary>
			/// <param name="tokens">The regular expressions.</param>
			/// <param name="_proc">Callback procedures.</param>
			RegexLexer(const collections::IEnumerable<WString>& tokens, RegexProc _proc);
			~RegexLexer();

			/// <summary>Tokenize a input text.</summary>
			/// <returns>The result.</returns>
			/// <param name="code">The text to tokenize.</param>
			/// <param name="codeIndex">Extra information that will store in [F:vl.regex.RegexToken.codeIndex].</param>
			RegexTokens									Parse(const WString& code, vint codeIndex=-1)const;
			/// <summary>Create a equivalence walker from this lexical analyzer.</summary>
			/// <returns>The walker.</returns>
			RegexLexerWalker							Walk()const;
			/// <summary>Create a equivalence colorizer from this lexical analyzer.</summary>
			/// <returns>The colorizer.</returns>
			RegexLexerColorizer							Colorize()const;
		};
	}
}

#endif