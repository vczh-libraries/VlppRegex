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

			bool operator<(CharRange item) const
			{
				return end < item.begin;
			}

			bool operator<=(CharRange item) const
			{
				return *this < item || *this == item;
			}

			bool operator>(CharRange item) const
			{
				return item.end < begin;
			}

			bool operator>=(CharRange item) const
			{
				return *this > item || *this == item;
			}

			bool operator==(CharRange item) const
			{
				return begin == item.begin && end == item.end;
			}

			bool operator!=(CharRange item) const
			{
				return begin != item.begin || item.end != end;
			}

			bool operator<(char32_t item) const
			{
				return end < item;
			}

			bool operator<=(char32_t item) const
			{
				return begin <= item;
			}

			bool operator>(char32_t item) const
			{
				return item < begin;
			}

			bool operator>=(char32_t item) const
			{
				return item <= end;
			}

			bool operator==(char32_t item) const
			{
				return begin <= item && item <= end;
			}

			bool operator!=(char32_t item) const
			{
				return item < begin || end < item;
			}
		};
	}
}

#endif