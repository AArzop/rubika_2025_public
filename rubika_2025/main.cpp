#include <SFML/Graphics.hpp>
#include <Imgui/imgui.h>
#include <Imgui/imgui-SFML.h>

#include "Profiler.h"
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/View.hpp>
#include <GlobalMgr.h>
#include <GameMgr.h>

unsigned long long uFrameCount = 0;

int main()
{
    sf::RenderWindow window(sf::VideoMode(GameMgr::GAME_SIZE_X * 2, GameMgr::GAME_SIZE_Y * 2), "SFML works!");
#ifdef _DEBUG
    ImGui::SFML::Init(window);
#endif

    sf::Clock clock;
    clock.restart();

    GlobalMgr::Instance()->Init();

    sf::View view(sf::Vector2f((float)GameMgr::GAME_SIZE_X / 2.f, (float)GameMgr::GAME_SIZE_Y / 2.f),
    sf::Vector2f(GameMgr::GAME_SIZE_X, GameMgr::GAME_SIZE_Y));
    window.setView(view);

    while (window.isOpen())
    {
        PROFILER_EVENT_BEGIN(PROFILER_COLOR_BLACK, "Frame %llu", uFrameCount);

        int deltaTimeMicroS = clock.getElapsedTime().asMicroseconds();
        float fDeltaTimeMS = (float)deltaTimeMicroS / 1000.f;
        float fDeltaTimeS = fDeltaTimeMS / 1000.f;
        sf::Time imGuiTime = clock.restart();

        PROFILER_EVENT_BEGIN(PROFILER_COLOR_BLUE, "Event & Input");

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }

#ifdef _DEBUG
            ImGui::SFML::ProcessEvent(window, event);
#endif
        }

        PROFILER_EVENT_END();

        PROFILER_EVENT_BEGIN(PROFILER_COLOR_RED, "Update");
#ifdef _DEBUG
        ImGui::SFML::Update(window, imGuiTime);
#endif

        GlobalMgr::Instance()->Update(fDeltaTimeS);

#ifdef _DEBUG
        GlobalMgr::Instance()->DrawDebug();
#endif

        PROFILER_EVENT_END();

        PROFILER_EVENT_BEGIN(PROFILER_COLOR_GREEN, "Draw");
        window.clear();

        GlobalMgr::Instance()->Draw(window);

#ifdef _DEBUG
        ImGui::SFML::Render(window);
#endif
        window.display();

        PROFILER_EVENT_END();

        PROFILER_EVENT_END();
        ++uFrameCount;
    }

#ifdef _DEBUG
    ImGui::SFML::Shutdown();
#endif

    GlobalMgr::Instance()->Delete();
    GlobalMgr::DeleteInstance();

    return 0;
}
