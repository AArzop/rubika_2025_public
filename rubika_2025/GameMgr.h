#pragma once

#include <SFML/Graphics/Sprite.hpp>
#include <vector>

class Entity;
class TextureData;

namespace sf
{
	class RenderWindow;
}

class GameMgr
{
public:
	void Init();
	void Update(float deltaTime);
	void Draw(sf::RenderWindow& window);
	void DrawDebug();
	void Delete();

	void Reset();

	static inline const unsigned GAME_SIZE_X = 480;
	static inline const unsigned GAME_SIZE_Y = 300;

	void SpawnEnemy();
private:
	void SpawnEnemy_Callback(const TextureData* texture, void* userData);

	struct Entity
	{
		sf::Vector2f Position;
		sf::Sprite Sprite;
	};

	std::vector<Entity*> Entities;
};