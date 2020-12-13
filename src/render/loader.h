#pragma once

#include "gle/gle.h"
#include <vector>
#include <unordered_map>


struct Texture {
	GLE::Texture2D texture;
};


struct Material {

	GLE::ShaderProgram program;
	std::unordered_map<std::string, GLE::Texture2D> textures;

};


struct Mesh {

	u32 vertexCount;
	GLE::VertexArray vao;
	GLE::VertexBuffer vbo;
	GLE::IndexBuffer ibo;
	u32 materialIndex;

};


struct ModelNode {

	std::vector<ModelNode> children;
	std::vector<u32> meshIndices;
	Mat4f baseTransform;
	bool visible;

};


struct Model {

	std::vector<Mesh> meshes;
	std::vector<Material> materials;
	ModelNode root;
	Mat4f transform;

};


class Uri;

namespace Loader {

	bool loadShader(GLE::ShaderProgram& program, const Uri& vsPath, const Uri& fsPath);
	bool loadTexture2D(GLE::Texture2D& texture, const Uri& path, bool flipY = false);
	bool loadArrayTexture2D(GLE::ArrayTexture2D& texture, const std::vector<Uri>& paths, bool flipY = false);
	bool loadCubemap(GLE::CubemapTexture& cubemap, const std::vector<Uri>& paths, bool flipY = false);
	bool loadModel(Model& model, const Uri& path, bool flipY = false);

}