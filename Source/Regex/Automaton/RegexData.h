/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_REGEX_REGEXDATA
#define VCZH_REGEX_REGEXDATA

#include <Vlpp.h>

namespace vl
{
	namespace regex_internal
	{

/***********************************************************************
CharRange
***********************************************************************/

		class CharRange
		{
		public:
			typedef collections::SortedList<CharRange>		List;

			char32_t				begin = 0;
			char32_t				end = 0;

			CharRange() = default;
			CharRange(char32_t _begin, char32_t _end) : begin(_begin), end(_end) {}

			std::partial_ordering operator<=>(const CharRange& cr)const
			{
				if (end < cr.begin) return std::partial_ordering::less;
				if (cr.end < begin) return std::partial_ordering::greater;
				return *this == cr ? std::partial_ordering::equivalent : std::partial_ordering::unordered;
			}

			bool operator==(const CharRange& cr)const
			{
				return begin == cr.begin && end == cr.end;
			}

			std::weak_ordering operator<=>(char32_t item)const
			{
				if (end < item) return std::weak_ordering::less;
				if (begin > item) return std::weak_ordering::greater;
				return std::weak_ordering::equivalent;
			}

			bool operator==(char32_t item)const
			{
				return begin <= item && item <= end;
			}
		};
	}
}

#endif