#include "GameMgr.h"

#include <TextureMgr.h>
#include <Enemy.h>
#include <GlobalMgr.h>
#include <RandomMgr.h>
#include <Direction.h>
#include <Player.h>

#ifdef _DEBUG

#include <Imgui/imgui.h>

#endif

GameMgr::GameMgr() : Round(0), MaxNumberOfEnemy(0), NumberOfEnemyLeft(0)
{}

void GameMgr::Init()
{
	Player* p = new Player();
	p->SetPosition(sf::Vector2f((float)GAME_SIZE_X / 2.f, (float)GAME_SIZE_Y / 2.f));

	TextureMgr& textureMgr = GlobalMgr::Instance()->GetTextureMgr();
	textureMgr.RequestLoadTexture("../resources/IsaacSprite.png", std::bind(&GameMgr::SpawnPlayer_Callback, this, std::placeholders::_1, std::placeholders::_2), p);

	MaxNumberOfEnemy = 25;
	NumberOfEnemyLeft = MaxNumberOfEnemy;
}

void GameMgr::Update(float deltaTime)
{
	if (LoseGame)
	{
		return;
	}

	if (Break)
	{
		BreakUpdate(deltaTime);
	}
	else
	{
		RoundUpdate(deltaTime);
	}

	for (Entity* e : Entities)
	{
		e->Update(deltaTime);
	}

	if (pPlayer)
	{
		pPlayer->Update(deltaTime);

		if (!InvicibleMode)
		{
			const sf::FloatRect& playerHitbox = pPlayer->GetHitbox();
			for (const Entity* entity : Entities)
			{
				const Enemy* e = dynamic_cast<const Enemy*>(entity);
				if (e)
				{
					const sf::FloatRect& eHitbox = e->GetHitbox();
					if (playerHitbox.intersects(eHitbox))
					{
						LoseGame = true;
					}
				}
			}
		}
	}
}

void GameMgr::Draw(sf::RenderWindow& window)
{
	for (Entity* e : Entities)
	{
		e->Draw(window);
	}

	if (pPlayer)
	{
		pPlayer->Draw(window);
	}
}

void GameMgr::Delete()
{
	for (Entity* e : Entities)
	{
		e->Delete();
		delete e;
	}

	Entities.clear();

	delete pPlayer;
	pPlayer = nullptr;
}

void GameMgr::Reset()
{}

void GameMgr::RequestEntityDeletion(const Entity* entity)
{
	for (auto it = Entities.begin(); it != Entities.end(); ++it)
	{
		if (*it == entity)
		{
			delete entity;
			Entities.erase(it);
		}
	}
}

void GameMgr::OnRoundStart()
{
	Break = false;
	TimeBetweenSpawn = 0.95 * TimeBetweenSpawn;
	MaxNumberOfEnemy = MaxNumberOfEnemy * 1.3;
	NumberOfEnemyLeft = MaxNumberOfEnemy;
	SpawnTimer = 0;
	BreakTime = 0;
	++Round;
}

void GameMgr::OnRoundEnd()
{
	Break = true;
}

void GameMgr::BreakUpdate(float deltaTime)
{
	BreakTime += deltaTime;
	if (BreakTime >= 5)
	{
		OnRoundStart();
	}
}

void GameMgr::RoundUpdate(float deltaTime)
{
	if (NumberOfEnemyLeft == 0)
	{
		OnRoundEnd();
		return;
	}

	SpawnTimer += deltaTime;
	if (SpawnTimer >= TimeBetweenSpawn)
	{
		SpawnEnemy();
	}
}

void GameMgr::SpawnEnemy()
{
	if (NumberOfEnemyLeft <= 0)
	{
		return;
	}

	--NumberOfEnemyLeft;

	RandomMgr& randMgr = GlobalMgr::Instance()->GetRandomMgr();


	eDirection dir = static_cast<eDirection>(randMgr.RandInt32(1, (int)eDirection::Count - 1));
	Enemy* e = new Enemy();

	sf::Vector2f pos;
	int spriteSize = e->GetSize().x;

	float dist = 80;
	switch (dir)
	{
	case eDirection::Up:
		pos.x = randMgr.RandNormalDouble(GAME_SIZE_X / 2.0, dist);
		pos.x = -e->GetSize().y;
		break;
	case eDirection::Down:
		pos.x = randMgr.RandNormalDouble(GAME_SIZE_X / 2.0, dist);
		pos.y = GAME_SIZE_Y;
		break;
	case eDirection::Left:
		pos.x = GAME_SIZE_X;
		pos.y = randMgr.RandNormalDouble(GAME_SIZE_Y / 2.0, dist);
		break;
	case eDirection::Right:
		pos.x = -e->GetSize().x;
		pos.y = randMgr.RandNormalDouble(GAME_SIZE_Y / 2.0, dist);
		break;
	case eDirection::None:
	case eDirection::Count:
	default:
		throw;
		break;
	}

	e->SetPosition(pos);
	e->SetDirection(dir);

	TextureMgr& textureMgr = GlobalMgr::Instance()->GetTextureMgr();

	int textureNb = randMgr.RandInt32(1, 6);

	char buf[128] = {};
	snprintf(buf, sizeof(buf), "../resources/enemy_texture_%d.png", textureNb);
	//snprintf(buf, sizeof(buf), "../resources/sample4k.jpg");

	textureMgr.RequestLoadTexture(buf, std::bind(&GameMgr::SpawnEnemy_Callback, this, std::placeholders::_1, std::placeholders::_2), e);

	SpawnTimer = randMgr.RandDouble(0, TimeBetweenSpawn);
}

void GameMgr::SpawnPlayer_Callback(const TextureData* textureData, void* userData)
{
	Player* p = reinterpret_cast<Player*>(userData);
	if (!p)
	{
		return;
	}

	if (textureData)
	{
		p->SetTextureAndAnimation(textureData->Path);
	}

	p->Init();
	pPlayer = p;
}

void GameMgr::SpawnEnemy_Callback(const TextureData* textureData, void* userData)
{
	Enemy* e = reinterpret_cast<Enemy*>(userData);
	if (!e)
	{
		return;
	}

	e->SetTextureAndAnimation(textureData->Path);
	e->Init();
	Entities.push_back(e);
}

#ifdef _DEBUG

void GameMgr::DrawDebug()
{
	
}

#endif