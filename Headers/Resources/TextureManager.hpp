#pragma once

#include <unordered_map>
#include <memory>

#include "Texture.hpp"

namespace Resources
{
	class TextureManager
	{
	public:
		TextureManager();

		~TextureManager() = default;

		Texture* GetTexture(std::string key);

		Texture* GetOrCreateTexture(std::string key);

		Texture* GetDefaultTexture();

		void EmplaceTexture(std::string& key, std::unique_ptr<Texture>&& tex);

	private:
		std::unordered_map<std::string, std::unique_ptr<Resources::Texture>> textures;
	};

}