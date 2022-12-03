/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#include "RegexExpression.h"

namespace vl
{
	namespace regex_internal
	{

/***********************************************************************
Helper Functions
***********************************************************************/

		bool IsChar(const char32_t*& input, char32_t c)
		{
			if (*input == c)
			{
				input++;
				return true;
			}
			else
			{
				return false;
			}
		}

		template<vint Size>
		bool IsChars(const char32_t*& input, const char32_t(&chars)[Size])
		{
			for (char32_t c : chars)
			{
				if (*input == c)
				{
					input++;
					return true;
				}
			}
			return false;
		}

		template<vint Size>
		bool IsStr(const char32_t*& input, const char32_t(&str)[Size])
		{
			for (vint i = 0; i < Size - 1; i++)
			{
				if (input[i] != str[i]) return false;
			}
			input += Size - 1;
			return true;
		}

		bool IsPositiveInteger(const char32_t*& input, vint& number)
		{
			bool readed = false;
			number = 0;
			while (U'0' <= *input && *input <= U'9')
			{
				number = number * 10 + (*input++) - U'0';
				readed = true;
			}
			return readed;
		}

		bool IsName(const char32_t*& input, U32String& name)
		{
			const char32_t* read = input;
			if ((U'A' <= *read && *read <= U'Z') || (U'a' <= *read && *read <= U'z') || *read == U'_')
			{
				read++;
				while ((U'A' <= *read && *read <= U'Z') || (U'a' <= *read && *read <= U'z') || (U'0' <= *read && *read <= U'9') || *read == U'_')
				{
					read++;
				}
			}
			if (input == read)
			{
				return false;
			}
			else
			{
				name = U32String::CopyFrom(input, vint(read - input));
				input = read;
				return true;
			}
		}

		Ptr<LoopExpression> ParseLoop(const char32_t*& input)
		{
			vint min = 0;
			vint max = 0;
			if (!*input)
			{
				return 0;
			}
			else if (IsChar(input, U'+'))
			{
				min = 1;
				max = -1;
			}
			else if (IsChar(input, U'*'))
			{
				min = 0;
				max = -1;
			}
			else if (IsChar(input, U'?'))
			{
				min = 0;
				max = 1;
			}
			else if (IsChar(input, U'{'))
			{
				if (IsPositiveInteger(input, min))
				{
					if (IsChar(input, U','))
					{
						if (!IsPositiveInteger(input, max))
						{
							max = -1;
						}
					}
					else
					{
						max = min;
					}
					if (!IsChar(input, U'}'))
					{
						goto THROW_EXCEPTION;
					}
				}
				else
				{
					goto THROW_EXCEPTION;
				}
			}
			else
			{
				return 0;
			}

			{
				auto expression = Ptr(new LoopExpression);
				expression->min = min;
				expression->max = max;
				expression->preferLong = !IsChar(input, U'?');
				return expression;
			}
		THROW_EXCEPTION:
			throw ArgumentException(L"Regular expression syntax error: Illegal loop expression.", L"vl::regex_internal::ParseLoop", L"input");
		}

		Ptr<Expression> ParseCharSet(const char32_t*& input)
		{
			if (!*input)
			{
				return 0;
			}
			else if (IsChar(input, U'^'))
			{
				return Ptr(new BeginExpression);
			}
			else if (IsChar(input, U'$'))
			{
				return Ptr(new EndExpression);
			}
			else if (IsChar(input, U'\\') || IsChar(input, U'/'))
			{
				auto expression = Ptr(new CharSetExpression);
				expression->reverse = false;
				switch (*input)
				{
				case U'.':
					expression->ranges.Add(CharRange(1, MaxChar32));
					break;
				case U'r':
					expression->ranges.Add(CharRange(U'\r', U'\r'));
					break;
				case U'n':
					expression->ranges.Add(CharRange(U'\n', U'\n'));
					break;
				case U't':
					expression->ranges.Add(CharRange(U'\t', U'\t'));
					break;
				case U'\\':case U'/':case U'(':case U')':case U'+':case U'*':case U'?':case U'|':
				case U'{':case U'}':case U'[':case U']':case U'<':case U'>':
				case U'^':case U'$':case U'!':case U'=':
					expression->ranges.Add(CharRange(*input, *input));
					break;
				case U'S':
					expression->reverse = true;
				case U's':
					expression->ranges.Add(CharRange(U' ', U' '));
					expression->ranges.Add(CharRange(U'\r', U'\r'));
					expression->ranges.Add(CharRange(U'\n', U'\n'));
					expression->ranges.Add(CharRange(U'\t', U'\t'));
					break;
				case U'D':
					expression->reverse = true;
				case U'd':
					expression->ranges.Add(CharRange(U'0', U'9'));
					break;
				case U'L':
					expression->reverse = true;
				case U'l':
					expression->ranges.Add(CharRange(U'_', U'_'));
					expression->ranges.Add(CharRange(U'A', U'Z'));
					expression->ranges.Add(CharRange(U'a', U'z'));
					break;
				case U'W':
					expression->reverse = true;
				case U'w':
					expression->ranges.Add(CharRange(U'_', U'_'));
					expression->ranges.Add(CharRange(U'0', U'9'));
					expression->ranges.Add(CharRange(U'A', U'Z'));
					expression->ranges.Add(CharRange(U'a', U'z'));
					break;
				default:
					throw ArgumentException(L"Regular expression syntax error: Illegal character escaping.", L"vl::regex_internal::ParseCharSet", L"input");
				}
				input++;
				return expression;
			}
			else if (IsChar(input, U'['))
			{
				auto expression = Ptr(new CharSetExpression);
				if (IsChar(input, U'^'))
				{
					expression->reverse = true;
				}
				else
				{
					expression->reverse = false;
				}
				bool midState = false;
				char32_t a = U'\0';
				char32_t b = U'\0';
				while (true)
				{
					if (IsChar(input, U'\\') || IsChar(input, U'/'))
					{
						char32_t c = U'\0';
						switch (*input)
						{
						case U'r':
							c = U'\r';
							break;
						case U'n':
							c = U'\n';
							break;
						case U't':
							c = U'\t';
							break;
						case U'-':case U'[':case U']':case U'\\':case U'/':case U'^':case U'$':
							c = *input;
							break;
						default:
							throw ArgumentException(L"Regular expression syntax error: Illegal character escaping, only \"rnt-[]\\/\" are legal escaped characters in [].", L"vl::regex_internal::ParseCharSet", L"input");
						}
						input++;
						midState ? b = c : a = c;
						midState = !midState;
					}
					else if (IsChars(input, U"-]"))
					{
						goto THROW_EXCEPTION;
					}
					else if (*input)
					{
						midState ? b = *input++ : a = *input++;
						midState = !midState;
					}
					else
					{
						goto THROW_EXCEPTION;
					}
					if (IsChar(input, U']'))
					{
						if (midState)
						{
							b = a;
						}
						if (!expression->AddRangeWithConflict(CharRange(a, b)))
						{
							goto THROW_EXCEPTION;
						}
						break;
					}
					else if (IsChar(input, U'-'))
					{
						if (!midState)
						{
							goto THROW_EXCEPTION;
						}
					}
					else
					{
						if (midState)
						{
							b = a;
						}
						if (expression->AddRangeWithConflict(CharRange(a, b)))
						{
							midState = false;
						}
						else
						{
							goto THROW_EXCEPTION;
						}
					}
				}
				return expression;
			THROW_EXCEPTION:
				throw ArgumentException(L"Regular expression syntax error: Illegal character set definition.");
			}
			else if (IsChars(input, U"()+*?{}|"))
			{
				input--;
				return 0;
			}
			else
			{
				auto expression = Ptr(new CharSetExpression);
				expression->reverse = false;
				expression->ranges.Add(CharRange(*input, *input));
				input++;
				return expression;
			}
		}

		Ptr<Expression> ParseFunction(const char32_t*& input)
		{
			if (IsStr(input, U"(="))
			{
				Ptr<Expression> sub = ParseExpression(input);
				if (!IsChar(input, U')'))
				{
					goto NEED_RIGHT_BRACKET;
				}
				auto expression = Ptr(new PositiveExpression);
				expression->expression = sub;
				return expression;
			}
			else if (IsStr(input, U"(!"))
			{
				Ptr<Expression> sub = ParseExpression(input);
				if (!IsChar(input, U')'))
				{
					goto NEED_RIGHT_BRACKET;
				}
				auto expression = Ptr(new NegativeExpression);
				expression->expression = sub;
				return expression;
			}
			else if (IsStr(input, U"(<&"))
			{
				U32String name;
				if (!IsName(input, name))
				{
					goto NEED_NAME;
				}
				if (!IsChar(input, U'>'))
				{
					goto NEED_GREATER;
				}
				if (!IsChar(input, U')'))
				{
					goto NEED_RIGHT_BRACKET;
				}
				auto expression = Ptr(new UsingExpression);
				expression->name = name;
				return expression;
			}
			else if (IsStr(input, U"(<$"))
			{
				U32String name;
				vint index = -1;
				if (IsName(input, name))
				{
					if (IsChar(input, U';'))
					{
						if (!IsPositiveInteger(input, index))
						{
							goto NEED_NUMBER;
						}
					}
				}
				else if (!IsPositiveInteger(input, index))
				{
					goto NEED_NUMBER;
				}
				if (!IsChar(input, U'>'))
				{
					goto NEED_GREATER;
				}
				if (!IsChar(input, U')'))
				{
					goto NEED_RIGHT_BRACKET;
				}
				auto expression = Ptr(new MatchExpression);
				expression->name = name;
				expression->index = index;
				return expression;
			}
			else if (IsStr(input, U"(<"))
			{
				U32String name;
				if (!IsName(input, name))
				{
					goto NEED_NAME;
				}
				if (!IsChar(input, U'>'))
				{
					goto NEED_GREATER;
				}
				auto sub = ParseExpression(input);
				if (!IsChar(input, U')'))
				{
					goto NEED_RIGHT_BRACKET;
				}
				auto expression = Ptr(new CaptureExpression);
				expression->name = name;
				expression->expression = sub;
				return expression;
			}
			else if (IsStr(input, U"(?"))
			{
				auto sub = ParseExpression(input);
				if (!IsChar(input, U')'))
				{
					goto NEED_RIGHT_BRACKET;
				}
				auto expression = Ptr(new CaptureExpression);
				expression->expression = sub;
				return expression;
			}
			else if (IsChar(input, U'('))
			{
				auto sub = ParseExpression(input);
				if (!IsChar(input, U')'))
				{
					goto NEED_RIGHT_BRACKET;
				}
				return sub;
			}
			else
			{
				return 0;
			}
		NEED_RIGHT_BRACKET:
			throw ArgumentException(L"Regular expression syntax error: \")\" expected.", L"vl::regex_internal::ParseFunction", L"input");
		NEED_GREATER:
			throw ArgumentException(L"Regular expression syntax error: \">\" expected.", L"vl::regex_internal::ParseFunction", L"input");
		NEED_NAME:
			throw ArgumentException(L"Regular expression syntax error: Identifier expected.", L"vl::regex_internal::ParseFunction", L"input");
		NEED_NUMBER:
			throw ArgumentException(L"Regular expression syntax error: Number expected.", L"vl::regex_internal::ParseFunction", L"input");
		}

		Ptr<Expression> ParseUnit(const char32_t*& input)
		{
			Ptr<Expression> unit = ParseCharSet(input);
			if (!unit)
			{
				unit = ParseFunction(input);
			}
			if (!unit)
			{
				return 0;
			}
			Ptr<LoopExpression> loop;
			while ((loop = ParseLoop(input)))
			{
				loop->expression = unit;
				unit = loop;
			}
			return unit;
		}

		Ptr<Expression> ParseJoin(const char32_t*& input)
		{
			auto expression = ParseUnit(input);
			while (true)
			{
				auto right = ParseUnit(input);
				if (right)
				{
					auto sequence = Ptr(new SequenceExpression);
					sequence->left = expression;
					sequence->right = right;
					expression = sequence;
				}
				else
				{
					break;
				}
			}
			return expression;
		}

		Ptr<Expression> ParseAlt(const char32_t*& input)
		{
			auto expression = ParseJoin(input);
			while (true)
			{
				if (IsChar(input, U'|'))
				{
					auto right = ParseJoin(input);
					if (right)
					{
						auto alternate = Ptr(new AlternateExpression);
						alternate->left = expression;
						alternate->right = right;
						expression = alternate;
					}
					else
					{
						throw ArgumentException(L"Regular expression syntax error: Expression expected.", L"vl::regex_internal::ParseAlt", L"input");
					}
				}
				else
				{
					break;
				}
			}
			return expression;
		}

		Ptr<Expression> ParseExpression(const char32_t*& input)
		{
			return ParseAlt(input);
		}

		Ptr<RegexExpression> ParseRegexExpression(const U32String& code)
		{
			auto regex = Ptr(new RegexExpression);
			const char32_t* start = code.Buffer();
			const char32_t* input = start;
			try
			{
				while (IsStr(input, U"(<#"))
				{
					U32String name;
					if (!IsName(input, name))
					{
						throw ArgumentException(L"Regular expression syntax error: Identifier expected.", L"vl::regex_internal::ParseRegexExpression", L"code");
					}
					if (!IsChar(input, U'>'))
					{
						throw ArgumentException(L"Regular expression syntax error: \">\" expected.", L"vl::regex_internal::ParseFunction", L"input");
					}
					Ptr<Expression> sub = ParseExpression(input);
					if (!IsChar(input, U')'))
					{
						throw ArgumentException(L"Regular expression syntax error: \")\" expected.", L"vl::regex_internal::ParseFunction", L"input");
					}
					if (regex->definitions.Keys().Contains(name))
					{
						throw ArgumentException(L"Regular expression syntax error: Found duplicated sub expression name: \"" + u32tow(name) + L"\". ", L"vl::regex_internal::ParseFunction", L"input");
					}
					else
					{
						regex->definitions.Add(name, sub);
					}
				}
				regex->expression = ParseExpression(input);
				if (!regex->expression)
				{
					throw ArgumentException(L"Regular expression syntax error: Expression expected.", L"vl::regex_internal::ParseUnit", L"input");
				}
				if (*input)
				{
					throw ArgumentException(L"Regular expression syntax error: Found unnecessary tokens.", L"vl::regex_internal::ParseUnit", L"input");
				}
				return regex;
			}
			catch (const ArgumentException& e)
			{
				throw RegexException(e.Message(), code, input - start);
			}
		}

		U32String EscapeTextForRegex(const U32String& literalString)
		{
			U32String result;
			for (vint i = 0; i < literalString.Length(); i++)
			{
				char32_t c = literalString[i];
				switch (c)
				{
				case U'\\':case U'/':case U'(':case U')':case U'+':case U'*':case U'?':case U'|':
				case U'{':case U'}':case U'[':case U']':case U'<':case U'>':
				case U'^':case U'$':case U'!':case U'=':
					result += U32String(U"\\") + U32String::FromChar(c);
					break;
				case U'\r':
					result += U"\\r";
					break;
				case U'\n':
					result += U"\\n";
					break;
				case U'\t':
					result += U"\\t";
					break;
				default:
					result += U32String::FromChar(c);
				}
			}
			return result;
		}

		U32String UnescapeTextForRegex(const U32String& escapedText)
		{
			U32String result;
			for (vint i = 0; i < escapedText.Length(); i++)
			{
				char32_t c = escapedText[i];
				if (c == U'\\' || c == U'/')
				{
					if (i < escapedText.Length() - 1)
					{
						i++;
						c = escapedText[i];
						switch (c)
						{
						case U'r':
							result += U"\r";
							break;
						case U'n':
							result += U"\n";
							break;
						case U't':
							result += U"\t";
							break;
						default:
							result += U32String::FromChar(c);
						}
						continue;
					}
				}
				result += U32String::FromChar(c);
			}
			return result;
		}

		U32String NormalizeEscapedTextForRegex(const U32String& escapedText)
		{
			U32String result;
			for (vint i = 0; i < escapedText.Length(); i++)
			{
				char32_t c = escapedText[i];
				if (c == U'\\' || c == U'/')
				{
					if (i < escapedText.Length() - 1)
					{
						i++;
						c = escapedText[i];
						result += U32String(U"\\") + U32String::FromChar(c);
						continue;
					}
				}
				result += U32String::FromChar(c);
			}
			return result;
		}

		bool IsRegexEscapedLiteralString(const U32String& regex)
		{
			for (vint i = 0; i < regex.Length(); i++)
			{
				char32_t c = regex[i];
				if (c == U'\\' || c == U'/')
				{
					i++;
				}
				else
				{
					switch (c)
					{
					case U'\\':case U'/':case U'(':case U')':case U'+':case U'*':case U'?':case U'|':
					case U'{':case U'}':case U'[':case U']':case U'<':case U'>':
					case U'^':case U'$':case U'!':case U'=':
						return false;
					}
				}
			}
			return true;
		}
	}
}