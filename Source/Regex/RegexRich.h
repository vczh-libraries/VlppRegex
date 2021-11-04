/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_REGEX_REGEXRICH
#define VCZH_REGEX_REGEXRICH

#include "./Automaton/RegexAutomaton.h"

namespace vl
{
	namespace regex_internal
	{
		class CaptureRecord
		{
		public:
			vint									capture;
			vint									start;
			vint									length;

			bool									operator==(const CaptureRecord& record)const;
		};
	}

	namespace regex_internal
	{
		class RichResult
		{
		public:
			vint									start;
			vint									length;
			collections::List<CaptureRecord>		captures;
		};

		class RichInterpretor : public Object
		{
		public:
		protected:
			class UserData
			{
			public:
				bool								NeedKeepState;
			};

			Automaton::Ref							dfa;
			UserData*								datas;
		public:
			RichInterpretor(Automaton::Ref _dfa);
			~RichInterpretor();

			template<typename TChar>
			bool									MatchHead(const TChar* input, const TChar* start, RichResult& result);

			template<typename TChar>
			bool									Match(const TChar* input, const TChar* start, RichResult& result);

			const collections::List<U32String>&		CaptureNames();
		};

		extern template bool	RichInterpretor::MatchHead<wchar_t>(const wchar_t* input, const wchar_t* start, RichResult& result);
		extern template bool	RichInterpretor::MatchHead<char8_t>(const char8_t* input, const char8_t* start, RichResult& result);
		extern template bool	RichInterpretor::MatchHead<char16_t>(const char16_t* input, const char16_t* start, RichResult& result);
		extern template bool	RichInterpretor::MatchHead<char32_t>(const char32_t* input, const char32_t* start, RichResult& result);

		extern template bool	RichInterpretor::Match<wchar_t>(const wchar_t* input, const wchar_t* start, RichResult& result);
		extern template bool	RichInterpretor::Match<char8_t>(const char8_t* input, const char8_t* start, RichResult& result);
		extern template bool	RichInterpretor::Match<char16_t>(const char16_t* input, const char16_t* start, RichResult& result);
		extern template bool	RichInterpretor::Match<char32_t>(const char32_t* input, const char32_t* start, RichResult& result);
	};
}

#endif