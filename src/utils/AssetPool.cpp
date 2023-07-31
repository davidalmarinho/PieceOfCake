#include "AssetPool.hpp"
#include <iostream>
#include <fstream>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

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

void AssetPool::insertModel(const std::string resourceID, const std::string MODEL_PATH)
{
	std::vector<Model::Vertex> vertices;
  std::vector<uint32_t> indices;

  // const std::string MODEL_PATH = "assets/models/viking_room.obj";

  /* The attrib container holds all of the positions, normals and texture 
   * coordinates in its attrib.vertices, attrib.normals and attrib.texcoords 
   * vectors. The shapes container contains all of the separate objects and 
   * their faces. Each face consists of an array of vertices, and each vertex 
   * contains the indices of the position, normal and texture coordinate attributes.
   */
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;

  if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str())) {
    throw std::runtime_error(warn + err);
  }

  std::unordered_map<Model::Vertex, uint32_t> uniqueVertices{};

  for (const auto& shape : shapes) {
    for (const auto& index : shape.mesh.indices) {
      Model::Vertex vertex{};

      vertex.pos = {
        // Multiply the index by 3 because the vertices is an array of positions
        // that is [X, Y, Z, X, Y, Z, X, Y, Z ...] and, by this way, we can access
        // to x, y and z coordinates.
        attrib.vertices[3 * index.vertex_index + 0],
        attrib.vertices[3 * index.vertex_index + 1],
        attrib.vertices[3 * index.vertex_index + 2]
      };

      // Flip the vertical component of the texture coordinates because 
      // we haveve uploaded the image into Vulkan in a top to bottom orientation.
      vertex.texCoords = {
        attrib.texcoords[2 * index.texcoord_index + 0],
        1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
      };

			vertex.color = {
				attrib.colors[3 * index.vertex_index + 0],
        attrib.colors[3 * index.vertex_index + 1],
        attrib.colors[3 * index.vertex_index + 2]
			};

      // vertex.color = {1.0f, 1.0f, 1.0f};

			vertex.normalCoords = {
				attrib.normals[3 * index.normal_index + 0],
				attrib.normals[3 * index.normal_index + 1],
				attrib.normals[3 * index.normal_index + 2]
				// 0,0,0
			};

      /**
       * Every time we read a vertex from the OBJ file, we will check if it was already 
       * seen a vertex with the exact same position and texture coordinates before. 
       * If not, we add it to vertices and store its index in the uniqueVertices container. 
       */
      if (uniqueVertices.count(vertex) == 0) {
        uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
        vertices.push_back(vertex);
      }

      indices.push_back(uniqueVertices[vertex]);
    }
  }

	std::shared_ptr<Model> model = std::make_shared<Model>(MODEL_PATH, vertices, indices);
	AssetPool::modelsMap.insert({ resourceID, model });
}

void AssetPool::addModel(const std::string resourceID, const std::string modelPath)
{
	// Add to hash map if it is empty --because if it is empty it is certain
	// that the texture hasn't been added yet.
	if (AssetPool::modelsMap.empty()) {
		AssetPool::insertModel(resourceID, modelPath);
		return;
	}

	// Check if the resouces ID is the same
	if (AssetPool::hasSameResourceID(resourceID, modelsMap)) return;

	// TODO: Abstract this --An idea might be creating an interface so there is the possibility of calling FileName.getFilepath();
	// Resource's ID wasn't the same. Checking for file names but only in debugging mode.
#ifndef NDEBUG
	for (auto mapObject : modelsMap) {
		if (mapObject.second->FILEPATH.compare(modelPath) == 0) {
			std::cout << "Warning: You the asset '" << modelPath << "', that has been already added before.\n";
		}
	}
#endif

	// The texture is different and must be added t the map.
	AssetPool::insertModel(resourceID, modelPath);
}

std::shared_ptr<Model> AssetPool::getModel(const std::string resourceID)
{
	auto mapObj = modelsMap.find(resourceID);
	return mapObj->second;
}

void AssetPool::loadTextures(VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue, VkCommandPool commandPool)
{
	for (auto mapObj : texturesMap) {
		mapObj.second->createTextureImage(device, physicalDevice, graphicsQueue, commandPool);
  	mapObj.second->createTextureImageView(device);
  	mapObj.second->createTextureSampler(device, physicalDevice);
	}
}

void AssetPool::loadModels()
{
	for (auto mpObj : modelsMap) {
		mpObj.second->init();
	}
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

void AssetPool::cleanModels()
{
	modelsMap.clear();
}

void AssetPool::cleanup()
{
	AssetPool::cleanTextures();
	AssetPool::cleanShaders();
	AssetPool::cleanModels();
}
