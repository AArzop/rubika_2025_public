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

struct TextureData
{
	std::filesystem::path Path;
	sf::Image Image;
	sf::Texture Texture;
	
	std::map<std::string, AnimationData> AnimationsData;
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

	// Vérifie que la texture ainsi que les fichiers de Metadata existent bien au path indiqué
	bool CheckTextureDependencies(const std::filesystem::path& texturePath);

	// On part du principe que CheckTextureDependencies a été appelé au préalable et donc qu'on peut load
	// la texture ainsi que mes fichiers de metadata en toute sécurité
	bool LoadTextureAndDependencies(const std::filesystem::path& texturePath);

	// Charge le fichier de metadata au path indiqué et ajoute le résultat du parsing
	// au textureData donné en paramètre
	// Ouvre le fichier et le parse avec RapidXml. Envoie le node d'Animation à la prochaine fonction
	bool LoadTextureMetadata(const std::filesystem::path& path, TextureData& textureData);

	// Recoit le node d'Animation de LoadTextureMetadata, parse toute les données compris dans le xml
	// et stocke le résultat dans TextureData donné en paramètre
	bool LoadAnimationMetadata(rapidxml::xml_node<>* node, TextureData& textureData);

	struct RequestData
	{
		RequestData(const std::filesystem::path& texturePath, TextureMgr* pMgr);
		sf::Thread Thread;
		std::vector<LoadCallback> Callbacks;
		std::vector<void*> UserData;
	};

	std::map<std::filesystem::path, TextureData> Textures;
	std::map<std::filesystem::path, RequestData> Requesting;
	std::set<std::filesystem::path> CallbacksNextFrame;

	mutable sf::Mutex TexturesMutex;
	mutable sf::Mutex CallbacksNextFrameMutex;
};