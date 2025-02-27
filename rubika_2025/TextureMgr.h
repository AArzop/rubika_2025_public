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

	// V�rifie que la texture ainsi que les fichiers de Metadata existent bien au path indiqu�
	bool CheckTextureDependencies(const std::filesystem::path& texturePath);

	// On part du principe que CheckTextureDependencies a �t� appel� au pr�alable et donc qu'on peut load
	// la texture ainsi que mes fichiers de metadata en toute s�curit�
	bool LoadTextureAndDependencies(const std::filesystem::path& texturePath);

	// Charge le fichier de metadata au path indiqu� et ajoute le r�sultat du parsing
	// au textureData donn� en param�tre
	// Ouvre le fichier et le parse avec RapidXml. Envoie le node d'Animation � la prochaine fonction
	bool LoadTextureMetadata(const std::filesystem::path& path, TextureData& textureData);

	// Recoit le node d'Animation de LoadTextureMetadata, parse toute les donn�es compris dans le xml
	// et stocke le r�sultat dans TextureData donn� en param�tre
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