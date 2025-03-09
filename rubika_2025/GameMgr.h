#pragma once

#include <vector>

class Entity;
class TextureData;
class Player;

namespace sf
{
	class RenderWindow;
}

class GameMgr
{
public:
	GameMgr();

	void Init();
	void Update(float deltaTime);
	void Draw(sf::RenderWindow& window);
	void Delete();

	void Reset();

	void RequestEntityDeletion(const Entity* entity);

	static inline const unsigned GAME_SIZE_X = 480;
	static inline const unsigned GAME_SIZE_Y = 300;

#ifdef _DEBUG
public:
	void DrawDebug();

private:
	bool InvicibleMode = false;

#else
	bool InvicibleMode = false;
#endif
private:
	void OnRoundStart();
	void OnRoundEnd();
	
	void BreakUpdate(float deltaTime);
	void RoundUpdate(float deltaTime);

	void SpawnEnemy();

	void SpawnPlayer_Callback(const TextureData* texture, void* userData);
	void SpawnEnemy_Callback(const TextureData* texture, void* userData);

	unsigned Round;
	unsigned MaxNumberOfEnemy;
	unsigned NumberOfEnemyLeft;
	float TimeBetweenSpawn = 0.55;
	float SpawnTimer = 0;

	bool Break;
	float BreakTime;

	std::vector<Entity*> Entities;

	Player* pPlayer = nullptr;
	bool LoseGame = false;
};