#include <cstring>

#include "Compare.hpp"

namespace Compare {
	bool strings(const char* s1, const char* s2)
	{
		if (strcmp(s1,s2) == 0) return true;
		return false;
	}
	
	bool strings(const std::string s1, const std::string s2)
	{
		if (s1.compare(s2) == 0) return true;
		return false;
	}
}
