#include "Resources/Texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <STB_Image/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <STB_Image/stb_image_write.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <filesystem>
#include <fstream>

#include "Networking/Serialization/Serializer.hpp"
#include "Networking/Serialization/Deserializer.hpp"

static const int WrapTable[] = { GL_REPEAT, GL_MIRRORED_REPEAT, GL_MIRROR_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER };
static const int FilterTable[] = { GL_NEAREST, GL_LINEAR };

static const char* ErrorStrings[] = {
	"Unknown error",
	"File not found",
	"File size is too big",
	"Image size is too small",
	"Image size is too big",
	"Image is invalid",
};

using namespace Resources;

Texture::Texture()
{
	filter = TextureFilterType::Linear;
	wrap = TextureWrapType::Repeat;
	loaded.Store(false);
}

Texture::~Texture()
{
	UnLoad();
}

const char* Resources::Texture::GetError(TextureError error)
{
	u8 id = static_cast<u8>(error);
	if (id >= 6) return ErrorStrings[0];
	return ErrorStrings[id];
}

const char* Resources::Texture::GetSTBIError()
{
	return stbi_failure_reason();
}

TextureError Resources::Texture::TryLoad(const char* pathIn, Texture* tex, Maths::Vec2 minSize, Maths::Vec2 maxSize, u64 maxFileSize)
{
	if (tex->IsLoaded())
	{
		tex->UnLoad();
	}
	std::filesystem::path p = pathIn;
	if (!std::filesystem::exists(p))
	{
		return TextureError::NO_FILE;
	}
	else if(std::filesystem::file_size(p) > maxFileSize)
	{
		return TextureError::FILE_TOO_BIG;
	}
	tex->fileType = p.extension().string();
	tex->path = pathIn;
	std::ifstream file(p, std::ios::ate | std::ios::binary);
	if (file.fail())
	{
		file.close();
		return TextureError::OTHER;
	}
	tex->dataSize = file.tellg();
	file.seekg(0, std::ios_base::beg);
	tex->FileData = new u8[tex->dataSize];
	file.read((char*)tex->FileData, tex->dataSize);
	file.close();
	int nrChannels;
	stbi_set_flip_vertically_on_load_thread(false);
	tex->ImageData = stbi_load_from_memory(tex->FileData, tex->dataSize, &tex->sizeX, &tex->sizeY, &nrChannels, 4);
	if (!tex->ImageData)
	{
		return TextureError::IMG_INVALID;
	}
	
	if (!(minSize == maxSize))
	{
		Maths::Vec2 res = Maths::Vec2((float)tex->GetTextureWidth(), (float)tex->GetTextureHeight());
		if (res.x < minSize.x || res.y < minSize.y)
		{
			stbi_image_free(tex->ImageData);
			tex->ImageData = nullptr;
			return TextureError::IMG_TOO_SMALL;
		}
		else if (res.x > maxSize.x || res.y > maxSize.y)
		{
			stbi_image_free(tex->ImageData);
			tex->ImageData = nullptr;
			return TextureError::IMG_TOO_BIG;
		}
	}
	tex->EndLoad();
	return TextureError::NONE;
}

bool Resources::Texture::PreLoad(Networking::Serialization::Deserializer& dr, const std::string& pathIn)
{
	if (loaded.Load()) UnLoad();
	if (FileData)
	{
		delete[] FileData;
		FileData = nullptr;
		receivedParts.clear();
	}
	if (!LargeFile::PreLoad(dr, pathIn)) return false;
	return (dr.Read(sizeX) && dr.Read(sizeY));
}

bool Resources::Texture::AcceptPacket(Networking::Serialization::Deserializer& dr)
{
	if (!LargeFile::AcceptPacket(dr)) return false;
	if (complete)
	{
		LoadFromMemory();
	}
	return true;
}

bool Resources::Texture::SerializeFile(Networking::Serialization::Serializer& sr) const
{
	if (!LargeFile::SerializeFile(sr)) return false;
	sr.Write(sizeX);
	sr.Write(sizeY);
	return true;
}

TextureError Resources::Texture::LoadFromMemory()
{
	if (loaded.Load())
	{
		UnLoad();
	}
	int nrChannels;
	Maths::IVec2 res = Maths::IVec2(sizeX, sizeY);
	stbi_set_flip_vertically_on_load_thread(false);
	ImageData = stbi_load_from_memory(FileData, dataSize, &sizeX, &sizeY, &nrChannels, 4);
	if (!ImageData)
	{
		return TextureError::IMG_INVALID;
	}
	if (res.x != sizeX || res.y != sizeY) // whatever happened here, something went wrong
	{
		stbi_image_free(ImageData);
		ImageData = nullptr;
		return TextureError::OTHER;
	}
	EndLoad();
	return TextureError::NONE;
}

Maths::Color4 Resources::Texture::ReadPixel(Maths::IVec2 pos) const
{
	if (!loaded.Load() || !ImageData || pos.x >= sizeX || pos.y >= sizeY)
		return Maths::Color4();
	size_t index = ((size_t)pos.x + ((size_t)sizeX - pos.y - 1) * sizeX) * 4;
	return Maths::Color4(ImageData[index], ImageData[index + 1], ImageData[index + 2], ImageData[index + 3]);
}

void Texture::SetFilterType(TextureFilterType in, bool bind)
{
	if (bind) glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GetFilterValue(in));
	filter = in;
}

void Texture::SetWrapType(TextureWrapType in, bool bind)
{
	if (bind) glBindTexture(GL_TEXTURE_2D, textureID);
	unsigned int value = GetWrapValue(in);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, value);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, value);
	wrap = in;
}

void Texture::Load(const char* pathIn)
{
	path = pathIn;
	if (loaded.Load()) return;
	int nrChannels;
	stbi_set_flip_vertically_on_load_thread(ShouldFlipTexture);
	ImageData = stbi_load(pathIn, &sizeX, &sizeY, &nrChannels, 4);
	if (!ImageData)
	{
		std::cout << "ERROR   : Could not load file " << pathIn << std::endl;
		std::cout << stbi_failure_reason() << std::endl;
	}
}

void Resources::Texture::EndLoad()
{
	if (!ImageData) return;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sizeX, sizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, ImageData);
	glGenerateMipmap(GL_TEXTURE_2D);

	if (ShouldDeleteData)
	{
		stbi_image_free(ImageData);
		ImageData = nullptr;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.f);

	loaded.Store(true);
}

void Resources::Texture::DeleteData()
{
	if (ImageData)
	{
		stbi_image_free(ImageData);
		ImageData = nullptr;
	}
	if (FileData)
	{
		delete FileData;
		FileData = nullptr;
	}
}

u64 Resources::Texture::GetTextureID() const
{
	return textureID;
}

unsigned int Resources::Texture::BindForRender(TextureFilterType FilterIn, TextureWrapType WrapIn)
{
	currentUnit = (currentUnit + 1) % TEXTURE_UPPER;
	glActiveTexture(GL_TEXTURE0 + currentUnit);
	glBindTexture(GL_TEXTURE_2D, textureID);
	if (FilterIn != filter) SetFilterType(FilterIn, false);
	if (WrapIn != wrap) SetWrapType(WrapIn, false);
	return currentUnit;
}

unsigned int Resources::Texture::BindForRender()
{
	return BindForRender(filter, wrap);
}

unsigned int Resources::Texture::IncrementGLUnit()
{
	currentCubeUnit = (currentCubeUnit + 1) % TEXTURE_UPPER;
	return currentCubeUnit + TEXTURE_UPPER;
}

unsigned int Resources::Texture::IncrementShadowGLUnit()
{
	currentCubeShadowUnit = (currentCubeShadowUnit + 1) % TEXTURE_UPPER;
	return currentCubeShadowUnit + SHADOWMAP_UPPER;
}

void Texture::UnLoad()
{
	if (textureID)
	{
		glDeleteTextures(1, &textureID);
	}
	DeleteData();
}

void Texture::Overwrite(const unsigned char* data, unsigned int sizeX, unsigned int sizeY)
{
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sizeX, sizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	this->sizeX = sizeX;
	this->sizeY = sizeY;
}

void Texture::SaveImage(const char* path, unsigned char* data, unsigned int sizeX, unsigned int sizeY)
{
	stbi_flip_vertically_on_write(true);
	std::string name = path;
	name.append("@");
	time_t timeLocal;
	struct tm dateTime;
	char text[64];
	time(&timeLocal);
	localtime_s(&dateTime, &timeLocal);
	strftime(text, 64, "%Y_%m_%d-%H_%M_%S", &dateTime);
	name.append(text);
	name.append(".png");
	if (!stbi_write_png(name.c_str(), sizeX, sizeY, 4, data, sizeX * 4))
	{
		std::cout << "ERROR   : Could not save file : " << path << std::endl;
		std::cout << stbi_failure_reason() << std::endl;
	}
	else
	{
		std::cout << "Saved screenshot as " << name.c_str() << std::endl;
	}
}

GLFWimage* Resources::Texture::ReadIcon(const char* path)
{
	int x, y, n;
	unsigned char* data = stbi_load(path, &x, &y, &n, 4);
	if (!data)
	{
		std::cout << "ERROR   : Could not load file "<< path << std::endl;
		std::cout << stbi_failure_reason() << std::endl;
		return nullptr;
	}
	GLFWimage* iconOut = new GLFWimage();
	iconOut->height = y;
	iconOut->width = x;
	iconOut->pixels = data;
	return iconOut;
}

void Resources::Texture::SetUnitLimit(int value)
{
	MAX_TEXTURE_UNIT = value;
	TEXTURE_UPPER = MAX_TEXTURE_UNIT / 4;
	CUBEMAP_UPPER = 2 * TEXTURE_UPPER;
	SHADOWMAP_UPPER = 3 * TEXTURE_UPPER;
	CUBESHADOWMAP_UPPER = 4 * TEXTURE_UPPER;
}

void Resources::Texture::SaveFileData(std::string& name)
{
	std::filesystem::path p = std::filesystem::current_path().append(name.append(fileType));
	std::ofstream file;
	file.open(p, std::ios_base::binary | std::ios_base::trunc | std::ios_base::out);
	if (!file.is_open() || file.fail())
	{
		std::string buf;
		buf.resize(512);
		_strerror_s(buf.data(), 512, nullptr);
		std::cout << buf.c_str() << std::endl;
		return;
	}
	file.write(reinterpret_cast<char*>(FileData), dataSize);
	file.close();
}

int Resources::Texture::GetFilterIndex(TextureFilterType type)
{
	return static_cast<int>(type);
}

int Resources::Texture::GetWrapIndex(TextureWrapType type)
{
	return static_cast<int>(type);
}

unsigned int Resources::Texture::GetWrapValue(TextureWrapType type)
{
	return WrapTable[GetWrapIndex(type)];
}

unsigned int Resources::Texture::GetFilterValue(TextureFilterType type)
{
	return FilterTable[GetFilterIndex(type)];
}

void Resources::Texture::SetCubeMap(int index)
{
	if (ImageData) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + index, 0, GL_RGBA, sizeX, sizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, ImageData);
		stbi_image_free(ImageData);
	}
	else
	{
		std::cout << "Cubemap texture failed to load " << std::endl;
	}
}
