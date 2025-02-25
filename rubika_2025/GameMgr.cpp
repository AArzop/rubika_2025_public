#include "GameMgr.h"

#include <SFML/Graphics/RenderWindow.hpp>

#include <TextureMgr.h>
#include <GlobalMgr.h>
#include <RandomMgr.h>

void GameMgr::Init()
{}

void GameMgr::Update(float deltaTime)
{}

void GameMgr::Draw(sf::RenderWindow& window)
{
	for (Entity* e : Entities)
	{
		sf::RenderStates states;
		states.transform.translate(e->Position);
		window.draw(e->Sprite, states);
	}
}

void GameMgr::DrawDebug()
{}

void GameMgr::Delete()
{
	for (Entity* e : Entities)
	{
		delete e;
	}

	Entities.clear();
}

void GameMgr::Reset()
{
	Delete();
}


void GameMgr::SpawnEnemy()
{
	RandomMgr& randMgr = GlobalMgr::Instance()->GetRandomMgr();

	Entity* e = new Entity();

	sf::Vector2f& pos = e->Position;

	float dist = 100;
	pos.x = randMgr.RandNormalDouble((float) GAME_SIZE_X / 2.0, (float)GAME_SIZE_X / 4.0);
	pos.y = randMgr.RandNormalDouble((float) GAME_SIZE_Y / 2.0, (float)GAME_SIZE_Y / 4.0);

	pos.x = std::clamp(pos.x, 0.f, (float)GAME_SIZE_X);
	pos.y = std::clamp(pos.y, 0.f, (float)GAME_SIZE_X);

	TextureMgr& textureMgr = GlobalMgr::Instance()->GetTextureMgr();
	int textureNb = randMgr.RandInt32(1, 5);

	char buf[128] = {};
	snprintf(buf, sizeof(buf), "../ressources/enemy_texture_%d.png", textureNb);

	textureMgr.RequestLoadTexture(buf, std::bind(&GameMgr::SpawnEnemy_Callback, this, std::placeholders::_1, std::placeholders::_2), e);
}

void GameMgr::SpawnEnemy_Callback(const TextureData* textureData, void* userData)
{
	Entity* e = reinterpret_cast<Entity*>(userData);
	if (!e)
	{
		return;
	}

	if (!textureData)
	{
		delete e;
		return;
	}

	e->Sprite.setTexture(textureData->Texture);
	Entities.push_back(e);
}
