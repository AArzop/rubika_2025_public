#pragma once

#include <SFML/Graphics/Texture.hpp>
#include <SFML/System.hpp>
#include <rapidxml/rapidxml.hpp>

#include <map>
#include <filesystem>
#include <vector>
#include <functional>
#include <set>

struct AnimationData
{
	AnimationData();

	int StartX;
	int StartY;
	int SizeX;
	int SizeY;
	int OffsetX;
	int OffsetY;
	int AnimationSpriteCount;
	int SpriteOnLine;
	bool IsReverted;
	float TimeBetweenAnimationInS;
};

struct StaticTileData
{
	int StartX;
	int StartY;
	int SizeX;
	int SizeY;
	bool IsRevertedX;
	bool IsRevertedY;
};

struct TextureData
{
	TextureData();
	~TextureData();

	std::filesystem::path Path;

	sf::Image Image;
	sf::Texture Texture;
	std::map<std::string, AnimationData> AnimationsData;
	std::map<std::string, StaticTileData> StaticTilesData;
};

class TextureMgr
{
public:

	using LoadCallback = std::function<void(const TextureData*, void* pUserData)>;

	void Init();
	void Update(float deltaTime);
	void Reset();

	const TextureData* GetTextureData(const std::filesystem::path& texturePath) const;
	TextureData* GetTextureData(const std::filesystem::path& texturePath);
	const sf::Texture* GetTexture(const std::filesystem::path& texturePath) const;
	void RequestLoadTexture(const std::filesystem::path& texturePath, LoadCallback callback, void* userData);

	void LoadTexture_Thread(std::filesystem::path texturePath);

#ifdef _DEBUG
	void DrawDebug();
#endif

private:

	bool CheckTextureDependencies(const std::filesystem::path& texturePath);
	bool LoadTextureAndDependencies(const std::filesystem::path& texturePath);
	bool LoadTextureMetadata(const std::filesystem::path& path, TextureData& textureData);
	bool LoadAnimationMetadata(rapidxml::xml_node<>* node, TextureData& textureData);
	bool LoadStaticTileMetadata(rapidxml::xml_node<>* node, TextureData& textureData);

	struct sLoadCallback
	{
		sLoadCallback(const std::filesystem::path& texturePath, TextureMgr* pMgr);
		sf::Thread Thread;
		std::vector<LoadCallback> Callbacks;
		std::vector<void*> UserData;
	};

	std::map<std::filesystem::path, TextureData> Textures;
	std::map<std::filesystem::path, sLoadCallback> Requesting;
	std::set<std::filesystem::path> CallbacksNextFrame;

	mutable sf::Mutex TexturesMutex;
	mutable sf::Mutex CallbacksNextFrameMutex;

	sf::Texture DefaultTexture;
};