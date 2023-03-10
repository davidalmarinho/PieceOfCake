#include <cstring>

#include "Compare.hpp"

namespace Compare {
	int strings(const char* s1, const char* s2)
	{
		if (strcmp(s1,s2) == 0) return 1;
		return 0;
	}
}
