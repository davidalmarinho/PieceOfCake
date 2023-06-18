#include "AssetPool.hpp"
#include <iostream>
#include <fstream>

// TODO: Make this a file's "database" manager.

//void AssetPool::addShader(const std::string shaderPath)
//{
	// Add to vector if it is empty --because if it is empty it is certain
	// that the shaderPath hasn't been added yet.
	//	if (AssetPool::shadersVec.empty()) {
	//		AssetPool::shadersVec.push_back(shaderPath);
	//	}

	//	// Add shaderPath to the vector if it hasn't been added yet.
	//	for (auto curString : AssetPool::shadersVec) {
	//		if (!Compare::strings(curString, shaderPath)) {
	//			AssetPool::shadersVec.push_back(shaderPath);
	//			break;
	//		}
	//	}
//}

// const std::string getShader(const std::string getShader)
// {
// 	
// }

std::vector<char> AssetPool::readFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("Error: Failed to open file!\n");
	}
	
	size_t fileSize = (size_t) file.tellg();
	std::vector<char> buffer(fileSize);

	// Seek back to the beginning of the file
	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}
