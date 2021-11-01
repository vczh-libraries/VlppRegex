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
Data Structure
***********************************************************************/

		class CharRange
		{
		public:
			typedef collections::SortedList<CharRange>		List;

			wchar_t					begin;
			wchar_t					end;

			CharRange();
			CharRange(wchar_t _begin, wchar_t _end);

			bool					operator<(CharRange item)const;
			bool					operator<=(CharRange item)const;
			bool					operator>(CharRange item)const;
			bool					operator>=(CharRange item)const;
			bool					operator==(CharRange item)const;
			bool					operator!=(CharRange item)const;

			bool					operator<(wchar_t item)const;
			bool					operator<=(wchar_t item)const;
			bool					operator>(wchar_t item)const;
			bool					operator>=(wchar_t item)const;
			bool					operator==(wchar_t item)const;
			bool					operator!=(wchar_t item)const;
		};
	}
}

#endif