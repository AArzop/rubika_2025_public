#pragma once

class TextureMgr;
class GameMgr;
class RandomMgr;

namespace sf
{
	class RenderWindow;
}

class GlobalMgr
{
	GlobalMgr();
	~GlobalMgr();

public:
	void Init();
	void Update(float deltaTime);
	void Draw(sf::RenderWindow& window);
	void Delete();

	static GlobalMgr* Instance();
	static void DeleteInstance();

	TextureMgr& GetTextureMgr() const;
	GameMgr& GetGameMgr() const;
	RandomMgr& GetRandomMgr() const;

#ifdef _DEBUG
public:
	void DrawDebug();
private:
	bool m_OpenDebugWindow = true;
#endif

private:

	TextureMgr* pTextureMgr;
	GameMgr* pGameMgr;
	RandomMgr* pRandomMgr;

	static inline GlobalMgr* sInstance;
};