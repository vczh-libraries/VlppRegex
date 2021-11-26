/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_REGEX_REGEXPURE
#define VCZH_REGEX_REGEXPURE

#include "./Automaton/RegexAutomaton.h"

namespace vl
{
	namespace stream
	{
		class IStream;
	}

	namespace regex_internal
	{
		class PureResult
		{
		public:
			vint				start;
			vint				length;
			vint				finalState;
			vint				terminateState;
		};

		class PureInterpretor : public Object
		{
		protected:
			static const vint	SupportedCharCount = MaxChar32 + 1;

			vint				charMap[SupportedCharCount];		// char -> char set index
			vint*				transitions = nullptr;				// (state * charSetCount + charSetIndex) -> state
			bool*				finalState = nullptr;				// state -> bool
			vint*				relatedFinalState = nullptr;		// sate -> (finalState or -1)
			vint				stateCount;
			vint				charSetCount;
			vint				startState;
		public:
			PureInterpretor(Automaton::Ref dfa, CharRange::List& subsets);
			PureInterpretor(stream::IStream& inputStream);
			~PureInterpretor();

			void				Serialize(stream::IStream& outputStream);

			template<typename TChar>
			bool				MatchHead(const TChar* input, const TChar* start, PureResult& result);

			template<typename TChar>
			bool				Match(const TChar* input, const TChar* start, PureResult& result);

			vint				GetStartState();
			vint				Transit(char32_t input, vint state);
			bool				IsFinalState(vint state);
			bool				IsDeadState(vint state);

			void				PrepareForRelatedFinalStateTable();
			vint				GetRelatedFinalState(vint state);
		};

		extern template bool	PureInterpretor::MatchHead<wchar_t>(const wchar_t* input, const wchar_t* start, PureResult& result);
		extern template bool	PureInterpretor::MatchHead<char8_t>(const char8_t* input, const char8_t* start, PureResult& result);
		extern template bool	PureInterpretor::MatchHead<char16_t>(const char16_t* input, const char16_t* start, PureResult& result);
		extern template bool	PureInterpretor::MatchHead<char32_t>(const char32_t* input, const char32_t* start, PureResult& result);

		extern template bool	PureInterpretor::Match<wchar_t>(const wchar_t* input, const wchar_t* start, PureResult& result);
		extern template bool	PureInterpretor::Match<char8_t>(const char8_t* input, const char8_t* start, PureResult& result);
		extern template bool	PureInterpretor::Match<char16_t>(const char16_t* input, const char16_t* start, PureResult& result);
		extern template bool	PureInterpretor::Match<char32_t>(const char32_t* input, const char32_t* start, PureResult& result);
	}
}

#endif