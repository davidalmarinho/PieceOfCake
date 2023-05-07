#pragma once

#include <vector>
#include <string>

class AssetPool
{
private:
	// inline static std::vector<std::string> shadersVec;

public:	
	// static void addShader(const std::string shaderPath);
	static std::vector<char> readFile(const std::string& filename);
};
