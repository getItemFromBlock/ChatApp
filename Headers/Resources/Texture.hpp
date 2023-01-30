#pragma once

#include <string>
#include <memory>

#include "Maths/Maths.hpp"
#include "Core/Types.hpp"
#include "Core/Signal.hpp"

struct GLFWimage;

enum class TextureFilterType : unsigned int
{
	Nearest,
	Linear,
};

enum class TextureWrapType : unsigned int
{
	Repeat,
	MirroredRepeat,
	MirroredClamp,
	ClampToEdge,
	ClampToBorder,
};

enum class TextureError : u8
{
	NONE = 0,
	NO_FILE,
	FILE_TOO_BIG,
	IMG_TOO_SMALL,
	IMG_TOO_BIG,
	IMG_INVALID,
	OTHER,
};

namespace Resources
{
	class Texture
	{
	public:

		Texture();
		~Texture();

		static const char* GetError(TextureError error);
		static const char* GetSTBIError();
		static TextureError TryLoad(const char* path, Texture* ptr, Maths::Vec2 minSize = Maths::Vec2(0,0), Maths::Vec2 maxSize = Maths::Vec2(0,0), u64 maxFileSize = -1);

		TextureError LoadFromMemory(u64 dataSize, unsigned char* dataIn, std::string& ext, std::string& p, Maths::IVec2 resolution);

		virtual void Load(const char* path);
		virtual void EndLoad();
		void Overwrite(const unsigned char* data, unsigned int sizeX, unsigned int sizeY);
		virtual void UnLoad();
		bool IsLoaded() const { return loaded.Load(); }

		void DeleteData();

		unsigned int GetTextureID() const;
		virtual unsigned int BindForRender();
		virtual unsigned int BindForRender(TextureFilterType FilterIn, TextureWrapType WrapIn);
		static unsigned int IncrementGLUnit();
		static unsigned int IncrementShadowGLUnit();
		int GetTextureWidth() const { return sizeX; }
		int GetTextureHeight() const { return sizeY; }
		Maths::Color4 ReadPixel(Maths::IVec2 pos) const;

		void SetFilterType(TextureFilterType in, bool bind = true);
		void SetWrapType(TextureWrapType in, bool bind = true);
		void SetShouldDeleteData(bool v) { ShouldDeleteData = v; }
		void SetShouldFlipTexture(bool v) { ShouldFlipTexture = v; }
		static int GetFilterIndex(TextureFilterType type);
		static int GetWrapIndex(TextureWrapType type);
		static unsigned int GetWrapValue(TextureWrapType type);
		static unsigned int GetFilterValue(TextureFilterType type);
		void SetCubeMap(int index);

		static void SaveImage(const char* path, unsigned char* data, unsigned int sizeX, unsigned int sizeY);
		static GLFWimage* ReadIcon(const char* path);
		static void SetUnitLimit(int value);

		u8* GetFileData() const { return FileData; }
		u64 GetFileDataSize() const { return dataSize; }
		u8* GetImageData() const { return ImageData; }

		const std::string& GetPath() const { return path; }
		const std::string& GetFileType() const { return fileType; }

		void SaveFileData(std::string& name);
	protected:
		unsigned int textureID = 0;
		int sizeX = 0;
		int sizeY = 0;
		TextureFilterType filter;
		TextureWrapType wrap;
		bool ShouldDeleteData = true;
		bool ShouldFlipTexture = false;
		unsigned char* ImageData = nullptr;
		unsigned char* FileData = nullptr;
		u64 dataSize = 0;
		Core::Signal loaded;
		std::string fileType;
		std::string path;

		static inline int MAX_TEXTURE_UNIT = 16;
		static inline int TEXTURE_UPPER = 4;
		static inline int CUBEMAP_UPPER = 8;
		static inline int SHADOWMAP_UPPER = 12;
		static inline int CUBESHADOWMAP_UPPER = 16;
		static inline int currentUnit = 0;
		static inline int currentCubeUnit = 0;
		static inline int currentShadowUnit = 0;
		static inline int currentCubeShadowUnit = 0;
	};
}