#include "AssetPool.hpp"
#include <iostream>
#include <fstream>

void AssetPool::addShader(const std::string resourceID, const std::string fragmentShaderPath, const std::string vertexShaderPath)
{
	// Add to hash map if it is empty --because if it is empty it is certain
	// that the shader path hasn't been added yet.
	if (AssetPool::shadersMap.empty()) {
		std::shared_ptr<Shader> shader = std::make_shared<Shader>(fragmentShaderPath, vertexShaderPath);
		AssetPool::shadersMap.insert({ resourceID, shader });
		return;
	}

	// Check if the resouces ID is the same
	auto mapObj = shadersMap.find(resourceID);
	if (mapObj != shadersMap.end()) {
		std::cout << "Warning: ResourceID '" << mapObj->first << "' has been already added before.\n";
		return;
	}

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
	
	// The shader wasn't be
	std::shared_ptr<Shader> shader = std::make_shared<Shader>(fragmentShaderPath, vertexShaderPath);
	AssetPool::shadersMap.insert({ resourceID, shader });
}

std::shared_ptr<Shader> AssetPool::getShader(const std::string resourceID)
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

void AssetPool::cleanup()
{
	AssetPool::cleanShaders();
}
