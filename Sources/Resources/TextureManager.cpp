#include "Resources/TextureManager.hpp"

using namespace Resources;

static const std::string defaultTex = std::string("Resources/DefaultUser.png");

TextureManager::TextureManager()
{
	std::unique_ptr<Texture> tex = std::make_unique<Texture>();
	tex->Load(defaultTex.c_str());
	tex->EndLoad();
	textures.emplace(defaultTex, std::move(tex));
}

Texture* TextureManager::GetTexture(std::string key)
{
	Texture* ptr;
	auto res = textures.find(key);
	if (res == textures.end())
	{
		ptr = GetDefaultTexture();
	}
	else
	{
		ptr = res->second.get();
	}
	return ptr;
}

Texture* TextureManager::GetOrCreateTexture(std::string key)
{
	Texture* ptr;
	auto res = textures.find(key);
	if (res == textures.end())
	{
		std::unique_ptr<Texture> tempTex = std::make_unique<Texture>();
		ptr = tempTex.get();
		textures.emplace(key, std::move(tempTex));
	}
	else
	{
		ptr = res->second.get();
	}
	return ptr;
}

Texture* TextureManager::GetDefaultTexture()
{
	return textures.at(defaultTex).get();
}

void Resources::TextureManager::EmplaceTexture(std::string& key, std::unique_ptr<Texture>&& tex)
{
	textures.emplace(key, std::move(tex));
}
