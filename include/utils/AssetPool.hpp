#pragma once

#include <vector>
#include <string>
#include <memory>
#include <map>
#include <iostream>

#include "Shader.hpp"
#include "Texture.hpp"

class AssetPool
{
private:
	inline static std::map<const std::string, std::shared_ptr<Shader>> shadersMap;
	inline static std::map<const std::string, std::shared_ptr<Texture>> texturesMap;
	static void cleanTextures();
	static void cleanShaders();
	static void insertShader(VkDevice device, const std::string resourceID, const std::string fragmentShaderPath, const std::string vertexShaderPath);
	static void insertTexture(VkDevice device, const std::string resourceID, const std::string texPath);

public:
	static void addShader(VkDevice device, const std::string resourceID, const std::string fragmentShaderPath, const std::string vertexShaderPath);
	static const std::shared_ptr<Shader> getShader(const std::string resourceID);
	static void addTexture(VkDevice device, const std::string resourceID, const std::string texPath);
	static const std::shared_ptr<Texture> getTexture(const std::string resourceID);
	static std::vector<char> readFile(const std::string& filename);

	static void cleanup();

	template <typename ValueType>
	static bool hasSameResourceID(const std::string resourceID, const std::map<const std::string, ValueType> myMap) {
		// Check if the resouces ID is the same
		auto mapObj = myMap.find(resourceID);
		if (mapObj != myMap.end()) {
			std::cout << "Warning: ResourceID '" << mapObj->first << "' has been already added before.\n";
			return true;
		}

		return false;
	}
};
