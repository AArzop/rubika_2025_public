#include "GlobalMgr.h"

#include <TextureMgr.h>
#include <GameMgr.h>
#include <RandomMgr.h>

#include <time.h>

GlobalMgr::GlobalMgr() : pTextureMgr(nullptr), pGameMgr(nullptr), pRandomMgr(nullptr)
{
    pTextureMgr = new TextureMgr();
    pGameMgr = new GameMgr();
    pRandomMgr = new RandomMgr();
}

GlobalMgr::~GlobalMgr()
{
    delete pTextureMgr;
    delete pGameMgr;
    delete pRandomMgr;
}

void GlobalMgr::Init()
{
    pTextureMgr->Init();
    pGameMgr->Init();

    srand(time(nullptr));
    pRandomMgr->SetSeed(rand());
}

void GlobalMgr::Update(float deltaTime)
{
    pTextureMgr->Update(deltaTime);
    pGameMgr->Update(deltaTime);
}

void GlobalMgr::Draw(sf::RenderWindow& window)
{
    pGameMgr->Draw(window);
}

void GlobalMgr::DrawDebug()
{
    pGameMgr->DrawDebug();
}

void GlobalMgr::Delete()
{
    pTextureMgr->Reset();
    pGameMgr->Reset();
}

GlobalMgr* GlobalMgr::Instance()
{
    if (!sInstance)
    {
        sInstance = new GlobalMgr();
    }

    return sInstance;
}

void GlobalMgr::DeleteInstance()
{
    if (sInstance)
    {
        delete sInstance;
        sInstance = nullptr;
    }
}

TextureMgr& GlobalMgr::GetTextureMgr() const
{
    return *pTextureMgr;
}

GameMgr& GlobalMgr::GetGameMgr() const
{
    return *pGameMgr;
}

RandomMgr& GlobalMgr::GetRandomMgr() const
{
    return *pRandomMgr;
}
