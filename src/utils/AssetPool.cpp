#include "AssetPool.hpp"
#include <iostream>
#include <fstream>

void AssetPool::insertShader(VkDevice device, const std::string resourceID, const std::string fragmentShaderPath, const std::string vertexShaderPath)
{
	std::shared_ptr<Shader> shader = std::make_shared<Shader>(device, fragmentShaderPath, vertexShaderPath);
	AssetPool::shadersMap.insert({ resourceID, shader });
}

void AssetPool::addShader(VkDevice device, const std::string resourceID, const std::string fragmentShaderPath, const std::string vertexShaderPath)
{
	// Add to hash map if it is empty --because if it is empty it is certain
	// that the shader path hasn't been added yet.
	if (AssetPool::shadersMap.empty()) {
		AssetPool::insertShader(device, resourceID, fragmentShaderPath, vertexShaderPath);
		return;
	}

	// Check if the resouces ID is the same
	if (AssetPool::hasSameResourceID(resourceID, shadersMap)) return;

	// TODO: abstract this.
	// Resource's ID wasn't the same. Checking for file names but only in debugging mode.
#ifndef NDEBUG
	for (auto mapObject : shadersMap) {
		if (mapObject.second->getFragmentShaderFilepath().compare(fragmentShaderPath) == 0) {
			std::cout << "Warning: You shouldn't have reloaded the shader '" << fragmentShaderPath << "' has been already added.\n";
		}
		if (mapObject.second->getVertexShaderFilepath().compare(vertexShaderPath) == 0) {
			std::cout << "Warning: You shouldn't have reloaded the shader '" << vertexShaderPath << "' has been already added.\n";
		}
	}
#endif
	
	// The shader is different and must be added t the map.
	AssetPool::insertShader(device, resourceID, fragmentShaderPath, vertexShaderPath);
}

const std::shared_ptr<Shader> AssetPool::getShader(const std::string resourceID)
{
	
	auto mapObj = shadersMap.find(resourceID);
	std::shared_ptr<Shader> shader = mapObj->second;
	return shader;
	/*for (std::shared_ptr<Shader> curShader : AssetPool::shadersMap) {
		bool shaderAlreadyExists = curShader->getFragmentShaderFilepath().compare(fragmentShaderPath) == 0 &&
															 curShader->getVertexShaderFilepath().compare(vertexShaderPath) == 0;
		
		if (shaderAlreadyExists) {
			return curShader;
		}
		else if (curShader->getFragmentShaderFilepath().compare(fragmentShaderPath)) {
			std::cout << "Warning: Are you sure about '" << vertexShaderPath << "' ?\n";
			return curShader;
		}
		else if (curShader->getVertexShaderFilepath().compare(vertexShaderPath)) {
			std::cout << "Warning: Are you sure about '" << fragmentShaderPath << "' ?\n";
			return curShader;
		}
	}

	// Means that the shader couldn't be found.
	std::shared_ptr<Shader> shader = std::make_shared<Shader>(fragmentShaderPath, vertexShaderPath);
	AssetPool::shadersVec.push_back(shader);
	return shader;*/
}

void AssetPool::insertTexture(VkDevice device, const std::string resourceID, const std::string texPath)
{
	std::shared_ptr<Texture> tex = std::make_shared<Texture>(device, texPath);
	AssetPool::texturesMap.insert({ resourceID, tex });
}

void AssetPool::addTexture(VkDevice device, const std::string resourceID, const std::string texPath)
{
	// Add to hash map if it is empty --because if it is empty it is certain
	// that the texture hasn't been added yet.
	if (AssetPool::texturesMap.empty()) {
		AssetPool::insertTexture(device, resourceID, texPath);
		return;
	}

	// Check if the resouces ID is the same
	if (AssetPool::hasSameResourceID(resourceID, texturesMap)) return;

	// TODO: Abstract this --An idea might be creating an interface so there is the possibility of calling FileName.getFilepath();
	// Resource's ID wasn't the same. Checking for file names but only in debugging mode.
#ifndef NDEBUG
	for (auto mapObject : texturesMap) {
		if (mapObject.second->getFilepath().compare(texPath) == 0) {
			std::cout << "Warning: You the asset '" << texPath << "', that has been already added before.\n";
		}
	}
#endif

	// The texture is different and must be added t the map.
	AssetPool::insertTexture(device, resourceID, texPath);
}
	
const std::shared_ptr<Texture> AssetPool::getTexture(const std::string resourceID)
{
	auto mapObj = texturesMap.find(resourceID);
	std::shared_ptr<Texture> tex = mapObj->second;
	return tex;
}

std::vector<char> AssetPool::readFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("Error: Failed to open file.\n");
	}
	
	size_t fileSize = (size_t) file.tellg();
	std::vector<char> buffer(fileSize);

	// Seek back to the beginning of the file
	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

void AssetPool::cleanShaders()
{
	shadersMap.clear();
}

void AssetPool::cleanTextures()
{
	texturesMap.clear();
}

void AssetPool::cleanup()
{
	AssetPool::cleanShaders();
}
