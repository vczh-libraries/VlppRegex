/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
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
		class RegexBase_;

/***********************************************************************
Data Structure
***********************************************************************/

		/// <summary>A sub string of the string that a <see cref="Regex"/> is matched against.</summary>
		/// <typeparam name="T>The character type.</typeparam>
		template<typename T>
		class RegexString_ : public Object
		{
		protected:
			ObjectString<T>								value;
			vint										start = 0;
			vint										length = 0;

		public:
			RegexString_() = default;
			RegexString_(vint _start) : start(_start) {}

			RegexString_(const ObjectString<T>& _string, vint _start, vint _length)
				: start(_start)
				, length(_length > 0 ? _length : 0)
			{
				if (_length > 0)
				{
					value = _string.Sub(_start, _length);
				}
			}

			/// <summary>The position of the input string in characters.</summary>
			/// <returns>The position.</returns>
			vint Start() const { return start; }
			/// <summary>The size of the sub string in characters.</summary>
			/// <returns>The size.</returns>
			vint Length() const { return length; }
			/// <summary>Get the sub string as a <see cref="U32String"/>.</summary>
			/// <returns>The sub string.</returns>
			const ObjectString<T>& Value() const { return value; }

			bool operator==(const RegexString_<T>& string) const
			{
				return start == string.start && length == string.length && value == string.value;
			}
		};

		/// <summary>A match produces by a <see cref="Regex"/>.</summary>
		/// <typeparam name="T>The character type.</typeparam>
		template<typename T>
		class RegexMatch_ : public Object
		{
			friend class RegexBase_;
		public:
			typedef Ptr<RegexMatch_<T>>										Ref;
			typedef collections::List<Ref>									List;
			typedef collections::List<RegexString_<T>>						CaptureList;
			typedef collections::Group<vint, RegexString_<T>>				CaptureGroup;
		protected:
			CaptureList														captures;
			CaptureGroup													groups;
			bool															success;
			RegexString_<T>													result;

			RegexMatch_(const ObjectString<T>& _string, regex_internal::PureResult* _result);
			RegexMatch_(const ObjectString<T>& _string, regex_internal::RichResult* _result);
			RegexMatch_(const RegexString_<T>& _result);
		public:
			NOT_COPYABLE(RegexMatch_<T>);
			
			/// <summary>
			/// Test if this match is a succeeded match or a failed match.
			/// A failed match will only appear when calling [M:vl.regex.Regex.Split] or [M:vl.regex.Regex.Cut].
			/// In other cases, failed matches are either not included in the result.
			/// </summary>
			/// <returns>Returns true if this match is a succeeded match.</returns>
			bool															Success()const;
			/// <summary>Get the matched sub string.</summary>
			/// <returns>The matched sub string.</returns>
			const RegexString_<T>&											Result()const;
			/// <summary>Get all sub strings that are captured anonymously.</summary>
			/// <returns>All sub strings that are captured anonymously.</returns>
			/// <example><![CDATA[
			/// int main()
			/// {
			///     Regex regex(L"^/.*?((?C/S+)(/.*?))+$");
			///     auto match = regex.MatchHead(L"C++ and C# are my favorite programing languages");
			///     for (auto capture : match->Captures())
			///     {
			///         Console::WriteLine(capture.Value());
			///     }
			/// }
			/// ]]></example>
			const CaptureList&												Captures()const;
			/// <summary>Get all sub strings that are captured by named groups.</summary>
			/// <returns>All sub strings that are captured by named groups.</returns>
			/// <example><![CDATA[
			/// int main()
			/// {
			///     Regex regex(L"^/.*?((<lang>C/S+)(/.*?))+$");
			///     auto match = regex.MatchHead(L"C++ and C# are my favorite programing languages");
			///     for (auto capture : match->Groups().Get(regex.CaptureNames().IndexOf(L"lang")))
			///     {
			///         Console::WriteLine(capture.Value());
			///     }
			/// }
			/// ]]></example>
			const CaptureGroup&												Groups()const;
		};

/***********************************************************************
Regex
***********************************************************************/

		class RegexBase_ : public Object
		{
		protected:
			regex_internal::PureInterpretor*			pure = nullptr;
			regex_internal::RichInterpretor*			rich = nullptr;

			template<typename T>
			void										Process(const ObjectString<T>& text, bool keepEmpty, bool keepSuccess, bool keepFail, typename RegexMatch_<T>::List& matches)const;
		public:
			RegexBase_() = default;
			~RegexBase_();

			/// <summary>Test is a DFA used to match a string.</summary>
			/// <returns>Returns true if a DFA is used.</returns>
			bool										IsPureMatch() const { return rich ? false : true; }
			/// <summary>Test is a DFA used to test a string. It ignores all capturing.</summary>
			/// <returns>Returns true if a DFA is used.</returns>
			bool										IsPureTest() const { return pure ? true : false; }

			/// <summary>Match a prefix of the text.</summary>
			/// <typeparam name="T>The character type of the text to match.</typeparam>
			/// <returns>Returns the match. Returns null if failed.</returns>
			/// <param name="text">The text to match.</param>
			/// <example><![CDATA[
			/// int main()
			/// {
			///     Regex regex(L"C/S+");
			///     auto match = regex.MatchHead(L"C++ and C# are my favorite programing languages");
			///     Console::WriteLine(match->Result().Value());
			/// }
			/// ]]></example>
			template<typename T>
			typename RegexMatch_<T>::Ref				MatchHead(const ObjectString<T>& text)const;
			template<typename T>
			typename RegexMatch_<T>::Ref				MatchHead(const T* text) const { return MatchHead<T>(ObjectString<T>(text)); }

			/// <summary>Match a sub string of the text.</summary>
			/// <typeparam name="T>The character type of the text to match.</typeparam>
			/// <returns>Returns the first match. Returns null if failed.</returns>
			/// <param name="text">The text to match.</param>
			/// <example><![CDATA[
			/// int main()
			/// {
			///     Regex regex(L"C/S+");
			///     auto match = regex.Match(L"C++ and C# are my favorite programing languages");
			///     Console::WriteLine(match->Result().Value());
			/// }
			/// ]]></example>
			template<typename T>
			typename RegexMatch_<T>::Ref				Match(const ObjectString<T>& text)const;
			template<typename T>
			typename RegexMatch_<T>::Ref				Match(const T* text) const { return Match<T>(ObjectString<T>(text)); }

			/// <summary>Match a prefix of the text, ignoring all capturing.</summary>
			/// <typeparam name="T>The character type of the text to match.</typeparam>
			/// <returns>Returns true if it succeeded.</returns>
			/// <param name="text">The text to match.</param>
			template<typename T>
			bool										TestHead(const ObjectString<T>& text)const;
			template<typename T>
			bool										TestHead(const T* text) const { return TestHead<T>(ObjectString<T>(text)); }

			/// <summary>Match a sub string of the text, ignoring all capturing.</summary>
			/// <typeparam name="T>The character type of the text to match.</typeparam>
			/// <returns>Returns true if succeeded.</returns>
			/// <param name="text">The text to match.</param>
			template<typename T>
			bool										Test(const ObjectString<T>& text)const;
			template<typename T>
			bool										Test(const T* text) const { return Test<T>(ObjectString<T>(text)); }

			/// <summary>Find all matched fragments in the given text, returning all matched sub strings.</summary>
			/// <typeparam name="T>The character type of the text to match.</typeparam>
			/// <param name="text">The text to match.</param>
			/// <param name="matches">Returns all succeeded matches.</param>
			/// <example><![CDATA[
			/// int main()
			/// {
			///     Regex regex(L"C/S+");
			///     RegexMatch::List matches;
			///     regex.Search(L"C++ and C# are my favorite programing languages", matches);
			///     for (auto match : matches)
			///     {
			///         Console::WriteLine(match->Result().Value());
			///     }
			/// }
			/// ]]></example>
			template<typename T>
			void										Search(const ObjectString<T>& text, typename RegexMatch_<T>::List& matches)const;
			template<typename T>
			void										Search(const T* text, typename RegexMatch_<T>::List& matches) const { return Search<T>(ObjectString<T>(text), matches); }

			/// <summary>Split the text by matched sub strings, returning all unmatched sub strings.</summary>
			/// <typeparam name="T>The character type of the text to match.</typeparam>
			/// <param name="text">The text to match.</param>
			/// <param name="keepEmptyMatch">Set to true to keep all empty unmatched sub strings. This could happen when there is nothing between two matched sub strings.</param>
			/// <param name="matches">Returns all failed matches.</param>
			/// <example><![CDATA[
			/// int main()
			/// {
			///     Regex regex(L"C/S+");
			///     RegexMatch::List matches;
			///     regex.Split(L"C++ and C# are my favorite programing languages", false, matches);
			///     for (auto match : matches)
			///     {
			///         Console::WriteLine(match->Result().Value());
			///     }
			/// }
			/// ]]></example>
			template<typename T>
			void										Split(const ObjectString<T>& text, bool keepEmptyMatch, typename RegexMatch_<T>::List& matches)const;
			template<typename T>
			void										Split(const T* text, bool keepEmptyMatch, typename RegexMatch_<T>::List& matches) const { return Split<T>(ObjectString<T>(text), keepEmptyMatch, matches); }

			/// <summary>Cut the text by matched sub strings, returning all matched and unmatched sub strings.</summary>
			/// <typeparam name="T>The character type of the text to match.</typeparam>
			/// <param name="text">The text to match.</param>
			/// <param name="keepEmptyMatch">Set to true to keep all empty matches. This could happen when there is nothing between two matched sub strings.</param>
			/// <param name="matches">Returns all succeeded and failed matches.</param>
			/// <example><![CDATA[
			/// int main()
			/// {
			///     Regex regex(L"C/S+");
			///     RegexMatch::List matches;
			///     regex.Cut(L"C++ and C# are my favorite programing languages", false, matches);
			///     for (auto match : matches)
			///     {
			///         Console::WriteLine(match->Result().Value());
			///     }
			/// }
			/// ]]></example>
			template<typename T>
			void										Cut(const ObjectString<T>& text, bool keepEmptyMatch, typename RegexMatch_<T>::List& matches)const;
			template<typename T>
			void										Cut(const T* text, bool keepEmptyMatch, typename RegexMatch_<T>::List& matches) const { return Cut<T>(ObjectString<T>(text), keepEmptyMatch, matches); }
		};

		/// <summary>
		/// <typeparam name="T>The character type of the regular expression itself.</typeparam>
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
		///     </ul>
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
		template<typename T>
		class Regex_ : public RegexBase_
		{
		protected:
			collections::List<ObjectString<T>>			captureNames;

			static U32String							ToU32(const ObjectString<T>& text);
			static ObjectString<T>						FromU32(const U32String& text);
		public:
			NOT_COPYABLE(Regex_<T>);

			/// <summary>Create a regular expression. It will crash if the regular expression produces syntax error.</summary>
			/// <param name="code">The regular expression in a string.</param>
			/// <param name="preferPure">Set to true to use DFA if possible.</param>
			Regex_(const ObjectString<T>& code, bool preferPure = true);
			~Regex_() = default;

			/// <summary>Get all names of named captures</summary>
			/// <returns>All names of named captures.</summary>
			const collections::List<ObjectString<T>>&	CaptureNames()const { return captureNames; }
		};

/***********************************************************************
Tokenizer
***********************************************************************/

		/// <summary>A token.</summary>
		/// <typeparam name="T>The character type.</typeparam>
		template<typename T>
		struct RegexToken_
		{
			/// <summary>Position in the input string in characters.</summary>
			vint										start;
			/// <summary>Size of this token in characters.</summary>
			vint										length;
			/// <summary>The token id, begins at 0, represents the regular expression in the list (the first argument in the contructor of <see cref="RegexLexer"/>) that matches this token. -1 means this token is produced by an error.</summary>
			vint										token;
			/// <summary>The pointer to where this token starts in the input string .</summary>
			/// <remarks>This pointer comes from a <see cref="U32String"/> that used to be analyzed. You should keep a variable to that string alive, so that to keep this pointer alive.</remarks>
			const T*									reading;
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

			bool operator==(const RegexToken_<T>& _token)const
			{
				return length == _token.length && token == _token.token && reading == _token.reading;
			}
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
			/// The length of the token, allowing to be updated by the callback.
			/// When the callback returns, the length is not allowed to be decreased.
			/// This value will be -1 if <see cref="interTokenState"/> is not null.
			/// </summary>
			vint										length;
			/// <summary>
			/// The id of the token, allowing to be updated by the callback.
			/// </summary>
			vint										token;
			/// <summary>
			/// The flag indicating if this token is completed, allowing to be updated by the callback.
			/// </summary>
			bool										completeToken;
			/// <summary>
			/// The inter token state object, allowing to be updated by the callback.
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
		template<typename T>
		using RegexTokenExtendProc = void(*)(void* argument, const T* reading, vint length, bool completeText, RegexProcessingToken& processingToken);
		using RegexTokenColorizeProc =  void(*)(void* argument, vint start, vint length, vint token);

		/// <summary>Callback procedures</summary>
		/// <typeparam name="T>The character type.</typeparam>
		template<typename T>
		struct RegexProc_
		{
			/// <summary>
			/// The deleter which deletes <see cref="RegexProcessingToken::interTokenState"/> created by <see cref="extendProc"/>.
			/// This callback is not called automatically.
			/// It is here to make the maintainance convenient for the caller.
			/// </summary>
			RegexInterTokenStateDeleter					deleter = nullptr;
			/// <summary>
			/// <p>The token extend callback. It is called after recognizing any token, and run a customized procedure to modify the token based on the given context.</p>
			/// <p>If the length parameter is -1, it means the caller does not measure the incoming text buffer, which automatically indicates that the buffer is null-terminated.</p>
			/// <p>If the length parameter is not -1, it means the number of available characters in the buffer.</p>
			/// <p>The completeText parameter could be true or false. When it is false, it means that the buffer does not contain all the text.</p>
			/// </summary>
			/// <remarks>
			/// <p>
			/// This is very useful to recognize any token that cannot be expressed using a regular expression.
			/// For example, a C++ literal string R"tag(the conteng)tag".
			/// It is recommended to add a token for <b>R"tag(</b>,
			/// and then use this extend proc to search for a <b>)tag"</b> to complete the token.
			/// </p>
			/// <p>
			/// <b>Important</b>:
			/// when colorizing a text line by line,
			/// a cross-line token could be incomplete at the end of the line.
			/// Because a given buffer ends at the end of that line,
			/// the extend proc is not able to know right now about what is going on in the future.
			/// Here is what <see cref="RegexProcessingToken::interTokenState"/> is designed for,
			/// the extend proc can store anything it wants using that pointer.
			/// </p>
			/// <p>
			/// The caller can get this pointer from the return value of <see cref="RegexLexerColorizer::Colorize"/>.
			/// This pointer only available for cross-line tokens, it is obvious that one line produces at most one such pointer.
			/// Then the caller keeps calling that function to walk throught the whole string.
			/// When the return value is changed, the pointer is no longer used, and it can be deleted by calling <see cref="deleter"/> manually.
			/// </p>
			/// <p>
			/// The first argument is <see cref="argument"/>.
			/// </p>
			/// <p>
			/// The second argument is a pointer to the buffer of the first character in this token.
			/// If the previous token is incomplete, then the buffer begins at the first character of the new buffer.
			/// </p>
			/// <p>
			/// The third argument is the length of the recognized token in characters.
			/// </p>
			/// <p>
			/// The fourth character indicates if the token is completed.
			/// Even if a token is completed, but the extend proc found that, the extend exceeds the end of the buffer,
			/// then it can update the value to make it incomplete.
			/// </p>
			/// <p>
			/// The fifth contains the context for this token. Fields except "start" are allowed to be updated by the extend proc.
			/// </p>
			/// </remarks>
			/// <example><![CDATA[
			/// int main()
			/// {
			///     List<WString> tokenDefs;
			///     tokenDefs.Add(L"/d+");
			///     tokenDefs.Add(L"[a-zA-Z_]/w*");
			///     tokenDefs.Add(L"\"([^\"/\\]|/\\/.)*\"");
			///     tokenDefs.Add(L"R\"[^(]*/(");
			///     tokenDefs.Add(L"[(){};]");
			///     tokenDefs.Add(L"/s+");
			///     tokenDefs.Add(L"///*+([^//*]|/*+[^//])*/*+//");
			/// 
			///     const wchar_t* lines[] = {
			///         L"/*********************",
			///         L"MAIN.CPP",
			///         L"*********************/",
			///         L"",
			///         L"int main()",
			///         L"{",
			///         L"    printf(\"This is a \\\"simple\\\" text.\");",
			///         L"    printf(R\"____(This is a",
			///         L"\"multiple lined\"",
			///         L"literal text)____\");",
			///         L"    return 0;",
			///         L"}",
			///     };
			/// 
			///     struct Argument
			///     {
			///         // for a real colorizer, you can put a color buffer here.
			///         // the buffer is reused for every line of code.
			///         // but for the demo, I put the current processing text instead.
			///         // so that I am able to print what is processed.
			///         const wchar_t* processingText = nullptr;
			///     } argument;
			/// 
			///     struct InterTokenState
			///     {
			///         WString postfix;
			///     };
			/// 
			///     RegexProc proc;
			///     proc.argument = &argument;
			///     proc.colorizeProc = [](void* argument, vint start, vint length, vint token)
			///     {
			///         // this is guaranteed by "proc.argument = &argument;"
			///         auto text = reinterpret_cast<Argument*>(argument)->processingText;
			///         Console::WriteLine(itow(token) + L": <" + WString(text + start, length) + L">");
			///     };
			///     proc.deleter = [](void* interTokenState)
			///     {
			///         delete reinterpret_cast<InterTokenState*>(interTokenState);
			///     };
			///     proc.extendProc = [](void* argument, const wchar_t* reading, vint length, bool completeText, RegexProcessingToken& processingToken)
			///     {
			///         // 3 is R"[^(]*/(
			///         // 7 is not used in tokenDefs, it is occupied to represent an extended literal string
			///         if (processingToken.token == 3 || processingToken.token == 7)
			///         {
			///             // for calling wcsstr, create a buffer that is zero terminated
			///             WString readingBuffer = length == -1 ? WString(reading, false) : WString(reading, length);
			///             reading = readingBuffer.Buffer();
			/// 
			///             // get the postfix, which is )____" in this case
			///             WString postfix;
			///             if (processingToken.interTokenState)
			///             {
			///                 postfix = reinterpret_cast<InterTokenState*>(processingToken.interTokenState)->postfix;
			///             }
			///             else
			///             {
			///                 postfix = L")" + WString(reading + 2, processingToken.length - 3) + L"\"";
			///             }
			/// 
			///             // try to find if the postfix, which is )____" in this case, appear in the given buffer
			///             auto find = wcsstr(reading, postfix.Buffer());
			///             if (find)
			///             {
			///                 // if we find the postfix, it means we find the end of the literal string
			///                 // here processingToken.token automatically becomes 7
			///                 // interTokenState needs to be nullptr to indicate this
			///                 processingToken.length = (vint)(find - reading) + postfix.Length();
			///                 processingToken.completeToken = true;
			///                 processingToken.interTokenState = nullptr;
			///             }
			///             else
			///             {
			///                 // if we don't find the postfix, it means the end of the literal string is in future lines
			///                 // we need to set the token to 7, which is the real token id for literal strings
			///                 // since we change any token from 3 to 7, 3 will never be passed to colorizeProc in "token" argument
			///                 processingToken.length = readingBuffer.Length();
			///                 processingToken.token = 7;
			///                 processingToken.completeToken = false;
			/// 
			///                 // we need to ensure that interTokenState is not nullptr, and we can save the postfix here
			///                 if (!completeText && !processingToken.interTokenState)
			///                 {
			///                     auto state = new InterTokenState;
			///                     state->postfix = postfix;
			///                     processingToken.interTokenState = state;
			///                 }
			///             }
			///         }
			///     };
			/// 
			///     RegexLexer lexer(tokenDefs, proc);
			///     RegexLexerColorizer colorizer = lexer.Colorize();
			/// 
			///     void* lastInterTokenState = nullptr;
			///     for (auto [line, index] : indexed(From(lines)))
			///     {
			///         Console::WriteLine(L"Begin line " + itow(index));
			///         argument.processingText = line;
			///         void* interTokenState = colorizer.Colorize(line, wcslen(line));
			///         
			///         if (lastInterTokenState && lastInterTokenState != interTokenState)
			///         {
			///             // call the deleter manually
			///             proc.deleter(lastInterTokenState);
			///         }
			///         lastInterTokenState = interTokenState;
			/// 
			///         argument.processingText = nullptr;
			///         colorizer.Pass(L'\r');
			///         colorizer.Pass(L'\n');
			///         Console::WriteLine(L"");
			///     }
			/// }
			/// ]]></example>
			RegexTokenExtendProc<T>						extendProc = nullptr;
			/// <summary>
			/// <p>
			/// The colorizer callback. It is called when a token is recognized.
			/// </p>
			/// <p>
			/// The first argument is <see cref="argument"/>.
			/// </p>
			/// <p>
			/// The second argument is the position of the first character of the token in characters.
			/// </p>
			/// <p>
			/// The third argument is the length of the recognized token in characters.
			/// </p>
			/// <p>
			/// The fourth character is the regular expression in the list (the first argument in the contructor of <see cref="RegexLexer"/>) that matches this token.
			/// </p>
			/// </summary>
			RegexTokenColorizeProc						colorizeProc = nullptr;
			/// <summary>
			/// The argument object that is the first argument for <see cref="extendProc"/> and <see cref="colorizeProc"/>.
			/// </summary>
			void*										argument = nullptr;
		};

		/// <summary>Token collection representing the result from the lexical analyzer. Call <see cref="RegexLexer::Parse"/> to create this object.</summary>
		/// <typeparam name="T>The character type.</typeparam>
		/// <example><![CDATA[
		/// int main()
		/// {
		///     List<WString> tokenDefs;
		///     tokenDefs.Add(L"/d+");
		///     tokenDefs.Add(L"/w+");
		///     tokenDefs.Add(L"/s+");
		/// 
		///     RegexLexer lexer(tokenDefs, {});
		///     WString input = L"I have 2 books.";
		///     auto tokenResult = lexer.Parse(input);
		/// 
		///     for (auto token : tokenResult)
		///     {
		///         // input must be in a variable
		///         // because token.reading points to a position from input.Buffer();
		///         Console::WriteLine(itow(token.token) + L": <" + WString(token.reading, token.length) + L">");
		///     }
		/// }
		/// ]]></example>
		template<typename T>
		class RegexTokens_ : public collections::EnumerableBase<RegexToken_<T>>
		{
			friend class RegexLexer;
		protected:
			regex_internal::PureInterpretor*			pure;
			const collections::Array<vint>&				stateTokens;
			U32String									code;
			vint										codeIndex;
			RegexProc_<T>								proc;
			
			RegexTokens_(regex_internal::PureInterpretor* _pure, const collections::Array<vint>& _stateTokens, const U32String& _code, vint _codeIndex, RegexProc_<T> _proc);
		public:
			RegexTokens_(const RegexTokens_<T>& tokens);
			~RegexTokens_() = default;
			
			collections::IEnumerator<RegexToken_<T>>*	CreateEnumerator() const override;

			/// <summary>Copy all tokens.</summary>
			/// <param name="tokens">Returns all tokens.</param>
			/// <param name="discard">A callback to decide which kind of tokens to discard. The input is [F:vl.regex.RegexToken.token]. Returns true to discard this kind of tokens.</param>
			/// <example><![CDATA[
			/// int main()
			/// {
			///     List<WString> tokenDefs;
			///     tokenDefs.Add(L"/d+");
			///     tokenDefs.Add(L"/w+");
			///     tokenDefs.Add(L"/s+");
			/// 
			///     RegexLexer lexer(tokenDefs, {});
			///     WString input = L"I have 2 books.";
			///     auto tokenResult = lexer.Parse(input);
			/// 
			///     List<RegexToken> filtered;
			///     tokenResult.ReadToEnd(filtered, [](vint token) { return token < 0 || token == 2; });
			/// 
			///     for (auto token : tokenResult)
			///     {
			///         // input must be in a variable
			///         // because token.reading points to a position from input.Buffer();
			///         Console::WriteLine(itow(token.token) + L": <" + WString(token.reading, token.length) + L">");
			///     }
			/// }
			/// ]]></example>
			void										ReadToEnd(collections::List<RegexToken_<T>>& tokens, bool(*discard)(vint)=0)const;
		};

/***********************************************************************
Template Instantiation
***********************************************************************/

		extern template class RegexString_<wchar_t>;
		extern template class RegexString_<char8_t>;
		extern template class RegexString_<char16_t>;
		extern template class RegexString_<char32_t>;

		extern template class RegexMatch_<wchar_t>;
		extern template class RegexMatch_<char8_t>;
		extern template class RegexMatch_<char16_t>;
		extern template class RegexMatch_<char32_t>;

		extern template RegexMatch_<wchar_t>::Ref	RegexBase_::MatchHead<wchar_t>	(const ObjectString<wchar_t>& text)const;
		extern template RegexMatch_<wchar_t>::Ref	RegexBase_::Match<wchar_t>		(const ObjectString<wchar_t>& text)const;
		extern template bool						RegexBase_::TestHead<wchar_t>	(const ObjectString<wchar_t>& text)const;
		extern template bool						RegexBase_::Test<wchar_t>		(const ObjectString<wchar_t>& text)const;
		extern template void						RegexBase_::Search<wchar_t>		(const ObjectString<wchar_t>& text, RegexMatch_<wchar_t>::List& matches)const;
		extern template void						RegexBase_::Split<wchar_t>		(const ObjectString<wchar_t>& text, bool keepEmptyMatch, RegexMatch_<wchar_t>::List& matches)const;
		extern template void						RegexBase_::Cut<wchar_t>		(const ObjectString<wchar_t>& text, bool keepEmptyMatch, RegexMatch_<wchar_t>::List& matches)const;

		extern template RegexMatch_<char8_t>::Ref	RegexBase_::MatchHead<char8_t>	(const ObjectString<char8_t>& text)const;
		extern template RegexMatch_<char8_t>::Ref	RegexBase_::Match<char8_t>		(const ObjectString<char8_t>& text)const;
		extern template bool						RegexBase_::TestHead<char8_t>	(const ObjectString<char8_t>& text)const;
		extern template bool						RegexBase_::Test<char8_t>		(const ObjectString<char8_t>& text)const;
		extern template void						RegexBase_::Search<char8_t>		(const ObjectString<char8_t>& text, RegexMatch_<char8_t>::List& matches)const;
		extern template void						RegexBase_::Split<char8_t>		(const ObjectString<char8_t>& text, bool keepEmptyMatch, RegexMatch_<char8_t>::List& matches)const;
		extern template void						RegexBase_::Cut<char8_t>		(const ObjectString<char8_t>& text, bool keepEmptyMatch, RegexMatch_<char8_t>::List& matches)const;

		extern template RegexMatch_<char16_t>::Ref	RegexBase_::MatchHead<char16_t>	(const ObjectString<char16_t>& text)const;
		extern template RegexMatch_<char16_t>::Ref	RegexBase_::Match<char16_t>		(const ObjectString<char16_t>& text)const;
		extern template bool						RegexBase_::TestHead<char16_t>	(const ObjectString<char16_t>& text)const;
		extern template bool						RegexBase_::Test<char16_t>		(const ObjectString<char16_t>& text)const;
		extern template void						RegexBase_::Search<char16_t>	(const ObjectString<char16_t>& text, RegexMatch_<char16_t>::List& matches)const;
		extern template void						RegexBase_::Split<char16_t>		(const ObjectString<char16_t>& text, bool keepEmptyMatch, RegexMatch_<char16_t>::List& matches)const;
		extern template void						RegexBase_::Cut<char16_t>		(const ObjectString<char16_t>& text, bool keepEmptyMatch, RegexMatch_<char16_t>::List& matches)const;

		extern template RegexMatch_<char32_t>::Ref	RegexBase_::MatchHead<char32_t>	(const ObjectString<char32_t>& text)const;
		extern template RegexMatch_<char32_t>::Ref	RegexBase_::Match<char32_t>		(const ObjectString<char32_t>& text)const;
		extern template bool						RegexBase_::TestHead<char32_t>	(const ObjectString<char32_t>& text)const;
		extern template bool						RegexBase_::Test<char32_t>		(const ObjectString<char32_t>& text)const;
		extern template void						RegexBase_::Search<char32_t>	(const ObjectString<char32_t>& text, RegexMatch_<char32_t>::List& matches)const;
		extern template void						RegexBase_::Split<char32_t>		(const ObjectString<char32_t>& text, bool keepEmptyMatch, RegexMatch_<char32_t>::List& matches)const;
		extern template void						RegexBase_::Cut<char32_t>		(const ObjectString<char32_t>& text, bool keepEmptyMatch, RegexMatch_<char32_t>::List& matches)const;

		extern template class Regex_<wchar_t>;
		extern template class Regex_<char8_t>;
		extern template class Regex_<char16_t>;
		extern template class Regex_<char32_t>;

		extern template class RegexTokens_<wchar_t>;
		extern template class RegexTokens_<char8_t>;
		extern template class RegexTokens_<char16_t>;
		extern template class RegexTokens_<char32_t>;
		
		using RegexString = RegexString_<wchar_t>;
		using RegexMatch = RegexMatch_<wchar_t>;
		using Regex = Regex_<wchar_t>;
		using RegexToken = RegexToken_<wchar_t>;
		using RegexProc = RegexProc_<wchar_t>;
		using RegexTokens = RegexTokens_<wchar_t>;

/***********************************************************************
RegexLexerWalker
***********************************************************************/
		
		/// <summary>A type for walking through a text against a <see cref="RegexLexer"/>. Call <see cref="RegexLexer::Walk"/> to create this object.</summary>
		/// <example><![CDATA[
		/// int main()
		/// {
		///     List<WString> tokenDefs;
		///     tokenDefs.Add(L"/d+./d+");
		///     tokenDefs.Add(L"/d+");
		///     tokenDefs.Add(L"/w+");
		///     tokenDefs.Add(L"/s+");
		/// 
		///     RegexLexer lexer(tokenDefs, {});
		///     RegexLexerWalker walker = lexer.Walk();
		/// 
		///     WString input = L"This book costs 2.5. That book costs 2.";
		///     const wchar_t* reading = input.Buffer();
		/// 
		///     const wchar_t* tokenBegin = reading;
		///     const wchar_t* tokenEnd = nullptr;
		///     vint tokenId = -1;
		/// 
		///     vint state = walker.GetStartState();
		///     while (*reading)
		///     {
		///         vint token = -1;
		///         bool finalState = false;
		///         bool previousTokenStop = false;
		///         walker.Walk(*reading++, state, token, finalState, previousTokenStop);
		/// 
		///         if (previousTokenStop || !*reading)
		///         {
		///             if (tokenEnd)
		///             {
		///                 if (tokenBegin == tokenEnd)
		///                 {
		///                     Console::WriteLine(L"Recognized token: " + itow(tokenId) + L": <" + WString(*tokenBegin) + L">");
		///                     tokenBegin = reading;
		///                     tokenEnd = nullptr;
		///                     tokenId = -1;
		///                     state = walker.GetStartState();
		///                 }
		///                 else
		///                 {
		///                     Console::WriteLine(L"Recognized token: " + itow(tokenId) + L": <" + WString(tokenBegin, tokenEnd - tokenBegin) + L">");
		///                     tokenBegin = reading = tokenEnd;
		///                     tokenEnd = nullptr;
		///                     tokenId = -1;
		///                     state = walker.GetStartState();
		///                 }
		///             }
		///             else
		///             {
		///                 Console::WriteLine(L"Unrecognized character: <" + WString(*tokenBegin) + L">");
		///                 tokenBegin++;
		///                 state = walker.GetStartState();
		///             }
		///         }
		///         else if (finalState)
		///         {
		///             tokenEnd = reading;
		///             tokenId = token;
		///         }
		///     }
		/// }
		/// ]]></example>
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
			/// <remarks>When calling <see cref="Walk"/> for the first character, the return value should be passed to the second parameter.</remarks>
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
			/// <param name="previousTokenStop">Returns true if the previous character is the end of the token.</param>
			/// <remarks>
			/// <p>
			/// The "finalState" argument is important.
			/// When "previousTokenStop" becomes true,
			/// it tells you that this character can no longer form a token with previous consumed characters.
			/// But it does not mean that the recognized token ends at the previous token.
			/// The recognized token could end eariler,
			/// which is indiated at the last time when "finalState" becomes true.
			/// </p>
			/// <p>
			/// See the example for <see cref="RegexLexerWalker"/> about how to use this function.
			/// </p>
			/// </remarks>
			void										Walk(char32_t input, vint& state, vint& token, bool& finalState, bool& previousTokenStop)const;
			/// <summary>Step forward by one character.</summary>
			/// <returns>Returns the new current state. It is used to walk the next character.</returns>
			/// <param name="input">The input character.</param>
			/// <param name="state">The current state.</param>
			vint										Walk(char32_t input, vint state)const;
			/// <summary>Test if the input text is a closed token.</summary>
			/// <returns>Returns true if the input text is a closed token.</returns>
			/// <param name="input">The input text.</param>
			/// <param name="length">Size of the input text in characters.</param>
			/// <remarks>
			/// <p>
			/// A closed token means that,
			/// there is a prefix that is a recognized token.
			/// At the same time, the input string itself could not be a token, or a prefix of any token.
			/// the recognized token has ended before reaching the end of the string.
			/// </p>
			/// <p>
			/// An unrecognized token is also considered as closed.
			/// </p>
			/// <p>
			/// For example, assume we have a token defined by "/d+./d+":
			/// <ul>
			///     <li>"2" is not a closed token, because it has not ended.</li>
			///     <li>
			///         "2.5." is a closed token, because it has ended at "2.5",
			///         and "2.5." could never be a prefix of any token,
			///         unless we have another token defined by "/d+./d+./d+".
			///     </li>
			/// </ul>
			/// </p>
			/// </remarks>
			/// <example><![CDATA[
			/// int main()
			/// {
			///     List<WString> tokenDefs;
			///     tokenDefs.Add(L"/d+./d+");
			///     tokenDefs.Add(L"/d+");
			/// 
			///     RegexLexer lexer(tokenDefs, {});
			///     RegexLexerWalker walker = lexer.Walk();
			/// 
			///     WString tests[] = { L".", L"2", L"2.", L"2.5", L"2.5." };
			///     for (auto test : From(tests))
			///     {
			///         if (walker.IsClosedToken(test.Buffer(), test.Length()))
			///         {
			///             Console::WriteLine(test + L" is a closed token.");
			///         }
			///         else
			///         {
			///             Console::WriteLine(test + L" is not a closed token.");
			///         }
			///     }
			/// }
			/// ]]></example>
			bool										IsClosedToken(const char32_t* input, vint length)const;
			/// <summary>Test if the input is a closed token.</summary>
			/// <returns>Returns true if the input text is a closed token.</returns>
			/// <param name="input">The input text.</param>
			/// <remarks>
			/// <p>
			/// A closed token means that,
			/// there is a prefix that is a recognized token.
			/// At the same time, the input string itself could not be a token, or a prefix of any token.
			/// the recognized token has ended before reaching the end of the string.
			/// </p>
			/// <p>
			/// An unrecognized token is also considered as closed.
			/// </p>
			/// <p>
			/// For example, assume we have a token defined by "/d+./d+":
			/// <ul>
			///     <li>"2" is not a closed token, because it has not ended.</li>
			///     <li>
			///         "2.5." is a closed token, because it has ended at "2.5",
			///         and "2.5." could never be a prefix of any token,
			///         unless we have another token defined by "/d+./d+./d+".
			///     </li>
			/// </ul>
			/// </p>
			/// </remarks>
			/// <example><![CDATA[
			/// int main()
			/// {
			///     List<WString> tokenDefs;
			///     tokenDefs.Add(L"/d+./d+");
			///     tokenDefs.Add(L"/d+");
			/// 
			///     RegexLexer lexer(tokenDefs, {});
			///     RegexLexerWalker walker = lexer.Walk();
			/// 
			///     WString tests[] = { L".", L"2", L"2.", L"2.5", L"2.5." };
			///     for (auto test : From(tests))
			///     {
			///         if (walker.IsClosedToken(test))
			///         {
			///             Console::WriteLine(test + L" is a closed token.");
			///         }
			///         else
			///         {
			///             Console::WriteLine(test + L" is not a closed token.");
			///         }
			///     }
			/// }
			/// ]]></example>
			bool										IsClosedToken(const U32String& input)const;
		};

/***********************************************************************
RegexLexerColorizer
***********************************************************************/

		/// <summary>Lexical colorizer. Call <see cref="RegexLexer::Colorize"/> to create this object.</summary>
		/// <example><![CDATA[
		/// int main()
		/// {
		///     List<WString> tokenDefs;
		///     tokenDefs.Add(L"/d+");
		///     tokenDefs.Add(L"[a-zA-Z_]/w*");
		///     tokenDefs.Add(L"[(){};]");
		///     tokenDefs.Add(L"/s+");
		///     tokenDefs.Add(L"///*+([^//*]|/*+[^//])*/*+//");
		/// 
		///     const wchar_t* lines[] = {
		///         L"/*********************",
		///         L"MAIN.CPP",
		///         L"*********************/",
		///         L"",
		///         L"int main()",
		///         L"{",
		///         L"    return 0;",
		///         L"}",
		///     };
		/// 
		///     struct Argument
		///     {
		///         // for a real colorizer, you can put a color buffer here.
		///         // the buffer is reused for every line of code.
		///         // but for the demo, I put the current processing text instead.
		///         // so that I am able to print what is processed.
		///         const wchar_t* processingText = nullptr;
		///     } argument;
		/// 
		///     RegexProc proc;
		///     proc.argument = &argument;
		///     proc.colorizeProc = [](void* argument, vint start, vint length, vint token)
		///     {
		///         // this is guaranteed by "proc.argument = &argument;"
		///         auto text = reinterpret_cast<Argument*>(argument)->processingText;
		///         Console::WriteLine(itow(token) + L": <" + WString(text + start, length) + L">");
		///     };
		/// 
		///     RegexLexer lexer(tokenDefs, proc);
		///     RegexLexerColorizer colorizer = lexer.Colorize();
		/// 
		///     for (auto [line, index] : indexed(From(lines)))
		///     {
		///         Console::WriteLine(L"Begin line " + itow(index));
		///         argument.processingText = line;
		///         colorizer.Colorize(line, wcslen(line));
		/// 
		///         argument.processingText = nullptr;
		///         colorizer.Pass(L'\r');
		///         colorizer.Pass(L'\n');
		///         Console::WriteLine(L"");
		///     }
		/// }
		/// ]]></example>
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

			void										CallExtendProcAndColorizeProc(const char32_t* input, vint length, RegexProcessingToken& token, bool colorize);
			vint										WalkOneToken(const char32_t* input, vint length, vint start, bool colorize);

			RegexLexerColorizer(const RegexLexerWalker& _walker, RegexProc _proc);
		public:
			RegexLexerColorizer(const RegexLexerColorizer& colorizer);
			~RegexLexerColorizer();

			/// <summary>Get the internal state.</summary>
			/// <returns>The internal state.</returns>
			/// <remarks>
			/// <p>
			/// If <see cref="Colorize"/> has not been called, the return value of this function is the start state.
			/// </p>
			/// <p>
			/// If a text is multi-lined, <see cref="Colorize"/> could be called line by line, and the internal state is changed.
			/// </p>
			/// <p>
			/// In order to colorize another piece of multi-lined text,
			/// you can either save the start state and call <see cref="SetInternalState"/> to reset the state,
			/// or call <see cref="RegexLexer::Colorize"/> for a new colorizer.
			/// </p>
			/// </remarks>
			InternalState								GetInternalState();
			/// <summary>Restore the colorizer to a specified state.</summary>
			/// <param name="state">The state to restore.</param>
			void										SetInternalState(InternalState state);
			/// <summary>Step forward by one character.</summary>
			/// <param name="input">The input character.</param>
			/// <remarks>Callbacks in <see cref="RegexProc"/> will be called <b>except colorizeProc</b>, which is from the second argument of the constructor of <see cref="RegexLexer"/>.</remarks>
			void										Pass(char32_t input);
			/// <summary>Get the start DFA state number, which represents the correct state before colorizing any characters.</summary>
			/// <returns>The DFA state number.</returns>
			vint										GetStartState()const;
			/// <summary>Colorize a text.</summary>
			/// <returns>An inter token state at the end of this line. It could be the same object to which is returned from the previous call.</returns>
			/// <param name="input">The text to colorize.</param>
			/// <param name="length">Size of the text in characters.</param>
			/// <remarks>
			/// <p>See <see cref="RegexProcessingToken::interTokenState"/> and <see cref="RegexProc::extendProc"/> for more information about the return value.</p>
			/// <p>Callbacks in <see cref="RegexProc"/> will be called, which is from the second argument of the constructor of <see cref="RegexLexer"/>.</p>
			/// </remarks>
			void*										Colorize(const char32_t* input, vint length);
		};

/***********************************************************************
RegexLexer
***********************************************************************/

		/// <summary>Lexical analyzer.</summary>
		class RegexLexer : public Object
		{
		protected:
			regex_internal::PureInterpretor*			pure = nullptr;
			collections::Array<vint>					ids;
			collections::Array<vint>					stateTokens;
			RegexProc									proc;

		public:
			NOT_COPYABLE(RegexLexer);
			/// <summary>Create a lexical analyzer by a set of regular expressions. [F:vl.regex.RegexToken.token] will be the index of the matched regular expression in the first argument.</summary>
			/// <param name="tokens">ALl regular expression, each one represent a kind of tokens.</param>
			/// <param name="_proc">Configuration of all callbacks.</param>
			RegexLexer(const collections::IEnumerable<U32String>& tokens, RegexProc _proc);
			~RegexLexer();

			/// <summary>Tokenize an input text.</summary>
			/// <returns>All tokens, including recognized tokens or unrecognized tokens. For unrecognized tokens, [F:vl.regex.RegexToken.token] will be -1.</returns>
			/// <param name="code">The text to tokenize.</param>
			/// <param name="codeIndex">Extra information that will be copied to [F:vl.regex.RegexToken.codeIndex].</param>
			/// <remarks>Callbacks in <see cref="RegexProc"/> will be called when iterating through tokens, which is from the second argument of the constructor of <see cref="RegexLexer"/>.</remarks>
			RegexTokens									Parse(const U32String& code, vint codeIndex=-1)const;
			/// <summary>Create a equivalence walker from this lexical analyzer. A walker enable you to walk throught characters one by one,</summary>
			/// <returns>The walker.</returns>
			RegexLexerWalker							Walk()const;
			/// <summary>Create a equivalence colorizer from this lexical analyzer.</summary>
			/// <returns>The colorizer.</returns>
			RegexLexerColorizer							Colorize()const;
		};
	}
}

#endif