#pragma once

#include <SFML/Graphics/Texture.hpp>
#include <SFML/System.hpp>
#include <rapidxml/rapidxml.hpp>

#include <map>
#include <filesystem>
#include <vector>
#include <functional>
#include <set>

struct TextureData
{
	std::filesystem::path Path;
	sf::Image Image;
	sf::Texture Texture;
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
private:

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
};