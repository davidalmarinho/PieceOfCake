#pragma once

#include <vector>
#include <string>
#include <memory>
#include <map>

#include "Shader.hpp"

class AssetPool
{
private:
	inline static std::map<const std::string, std::shared_ptr<Shader>> shadersMap;
	static void cleanShaders();

public:
	static void addShader(VkDevice device, const std::string resourceID, const std::string fragmentShaderPath, const std::string vertexShaderPath);
	static std::shared_ptr<Shader> getShader(const std::string resourceID);
	static std::vector<char> readFile(const std::string& filename);
	static void cleanup();
};
