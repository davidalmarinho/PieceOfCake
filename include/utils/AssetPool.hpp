#pragma once

#include <vector>
#include <string>
#include <memory>
#include <map>
#include <iostream>

#include "Shader.hpp"
#include "Texture.hpp"
#include "Model.hpp"

class AssetPool
{
private:
	inline static std::map<const std::string, std::shared_ptr<Shader>> shadersMap;
	inline static std::map<const std::string, std::shared_ptr<Texture>> texturesMap;
	inline static std::map<const std::string, std::shared_ptr<Model>> modelsMap;
	static void cleanTextures();
	static void cleanShaders();
	static void cleanModels();
	static void insertShader(VkDevice device, const std::string resourceID, const std::string fragmentShaderPath, const std::string vertexShaderPath);
	static void insertTexture(VkDevice device, const std::string resourceID, const std::string texPath);
	static void insertModel(const std::string resouceID, const std::string modelPath);

public:
	static void addShader(VkDevice device, const std::string resourceID, const std::string fragmentShaderPath, const std::string vertexShaderPath);
	static const std::shared_ptr<Shader> getShader(const std::string resourceID);
	static void addTexture(VkDevice device, const std::string resourceID, const std::string texPath);
	static const std::shared_ptr<Texture> getTexture(const std::string resourceID);
	static void addModel(const std::string resourceID, const std::string modelPath);

	static void loadModels();

	/** TODO: This description is outdated.
	 * @brief Gets the Model object by reference stored in its std::map.
	 * This function doesn't take ownership responsibility.
	 * So you shouldn't delete the object, as it is still owned by the original owner.
	 * Avoid holding raw pointers to the same Model objects returned by 
	 * the AssetPool.
	 *
	 * Correct use:
	 * 
	 * Model& model = AssetPool::getModel("model");
	 * model.bind(commandBuffer); 
	 *
	 * Incorret use:
	 * 
	 * Model& model = AssetPool::getModel("model");
	 * // ...
	 * SomeOtherFunction(&model); // Passing a raw pointer to the model to another function.
	 * 
	 * @param resourceID Resource's alias so you don't have to write all the filepath.
	 * @return Model& Returns the model object by reference stored in its std::map.
	 */
	// TODO: Maybe you want a weak pointer instead of a shared pointer.
	static std::shared_ptr<Model> getModel(const std::string resourceID);

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
