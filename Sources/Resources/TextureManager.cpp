#include "Resources/TextureManager.hpp"

using namespace Resources;

static const std::string defaultUserTexStr = std::string("Resources/DefaultUser.png");
static const std::string defaultImageTexStr = std::string("Resources/EmptyImage.png");
static const std::string defaultDownloadTexStr = std::string("Resources/UnloadedImage.png");

static Texture* defaultUserTex = nullptr;
static Texture* defaultImageTex = nullptr;
static Texture* defaultDownloadTex = nullptr;

TextureManager::TextureManager()
{
	std::unique_ptr<Texture> tex1 = std::make_unique<Texture>();
	tex1->Load(defaultUserTexStr.c_str());
	tex1->EndLoad();
	defaultUserTex = tex1.get();
	textures.emplace(defaultUserTexStr, std::move(tex1));
	std::unique_ptr<Texture> tex2 = std::make_unique<Texture>();
	tex2->Load(defaultImageTexStr.c_str());
	tex2->EndLoad();
	defaultImageTex = tex2.get();
	textures.emplace(defaultImageTexStr, std::move(tex2));
	std::unique_ptr<Texture> tex3 = std::make_unique<Texture>();
	tex3->Load(defaultDownloadTexStr.c_str());
	tex3->EndLoad();
	defaultDownloadTex = tex3.get();
	textures.emplace(defaultDownloadTexStr, std::move(tex3));
}

Texture* TextureManager::GetTexture(std::string key)
{
	Texture* ptr;
	auto res = textures.find(key);
	if (res == textures.end())
	{
		ptr = GetDefaultImage();
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

Texture* TextureManager::GetDefaultUserTexture()
{
	return defaultUserTex;
}

Texture* Resources::TextureManager::GetDefaultImage()
{
	return defaultImageTex;
}

Texture* Resources::TextureManager::GetLoadingImage()
{
	return defaultDownloadTex;
}

void Resources::TextureManager::EmplaceTexture(std::string& key, std::unique_ptr<Texture>&& tex)
{
	textures.emplace(key, std::move(tex));
}
