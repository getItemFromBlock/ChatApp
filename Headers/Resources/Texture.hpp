#pragma once



#include "LargeFile.hpp"
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
	class Texture : public LargeFile
	{
	public:

		Texture();
		virtual ~Texture() override;

		static const char* GetError(TextureError error);
		static const char* GetSTBIError();
		static TextureError TryLoad(const char* path, Texture* ptr, Maths::Vec2 minSize = Maths::Vec2(0,0), Maths::Vec2 maxSize = Maths::Vec2(0,0), u64 maxFileSize = -1);

		virtual bool PreLoad(Networking::Serialization::Deserializer& dr, const std::string& path) override;
		virtual bool AcceptPacket(Networking::Serialization::Deserializer& dr) override;
		virtual bool SerializeFile(Networking::Serialization::Serializer& sr) const override;
		TextureError LoadFromMemory();
		TextureError GetLastError() { return lastError; }

		virtual void Load(const char* path);
		virtual void EndLoad();
		void Overwrite(const unsigned char* data, unsigned int sizeX, unsigned int sizeY);
		virtual void UnLoad();
		bool IsLoaded() const { return loaded.Load(); }

		void DeleteData();

		u64 GetTextureID() const;
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

		void SaveFileData(std::string& name);
	protected:
		unsigned int textureID = 0;
		s32 sizeX = 0;
		s32 sizeY = 0;
		TextureFilterType filter;
		TextureWrapType wrap;
		bool ShouldDeleteData = true;
		bool ShouldFlipTexture = false;
		bool IsTextureLoading = false;
		u8* ImageData = nullptr;
		Core::Signal loaded;
		TextureError lastError = TextureError::NONE;

		static inline s32 MAX_TEXTURE_UNIT = 16;
		static inline s32 TEXTURE_UPPER = 4;
		static inline s32 CUBEMAP_UPPER = 8;
		static inline s32 SHADOWMAP_UPPER = 12;
		static inline s32 CUBESHADOWMAP_UPPER = 16;
		static inline s32 currentUnit = 0;
		static inline s32 currentCubeUnit = 0;
		static inline s32 currentShadowUnit = 0;
		static inline s32 currentCubeShadowUnit = 0;
	};
}