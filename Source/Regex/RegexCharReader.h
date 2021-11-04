/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_REGEX_REGEXCHARREADER
#define VCZH_REGEX_REGEXCHARREADER

#include <Vlpp.h>

namespace vl
{
	namespace regex_internal
	{
		template<typename T>
		struct CharReader
		{
		private:
			encoding::UtfStringTo32Reader<T>	reader;
			const T*							input;

		public:
			CharReader(const T* _input)
				: reader(_input)
				, input(_input)
			{
			}

			const T* Reading() { return input + reader.SourceCluster().index; }
			vint Index() { return reader.SourceCluster().index; }

			char32_t Read()
			{
				return reader.Read();
			}
		};

		template<>
		struct CharReader<char32_t>
		{
		private:
			const char32_t*						input;
			vint								index = 0;
			bool								finished = false;
		public:
			CharReader(const char32_t* _input)
				: input(_input)
			{
			}

			char32_t Read()
			{
				if (finished) return 0;
				if (auto c = input[index])
				{
					index++;
					return c;
				}
				else
				{
					finished = true;
					return 0;
				}
			}

			const char32_t* Reading() { return input + Index(); }
			vint Index() { return finished ? index : index - 1; }
		};
	}
}

#endif