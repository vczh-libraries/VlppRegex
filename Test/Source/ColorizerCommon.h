#include "../../Source/Regex/Regex.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::regex;

inline void ColorizerProc(void* argument, vint start, vint length, vint token)
{
	vint* colors = (vint*)argument;
	for (vint i = 0; i < length; i++)
	{
		colors[start + i] = token;
	}
}

template<int Size, int Length, typename T>
inline void* AssertColorizer(vint(&actual)[Size], vint(&expect)[Length], RegexLexerColorizer_<T>& colorizer, const T(&input)[Length + 1], bool firstLine)
{
	for (vint i = 0; i < Size; i++)
	{
		actual[i] = -2;
	}
	auto newStateObject = colorizer.Colorize(input, Length);
	for (vint i = 0; i < Length; i++)
	{
		TEST_ASSERT(actual[i] == expect[i]);
	}
	return newStateObject;
}
