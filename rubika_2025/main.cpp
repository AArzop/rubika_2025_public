#include <SFML/Graphics.hpp>
#include <Imgui/imgui.h>
#include <Imgui/imgui-SFML.h>

#include "Profiler.h"
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/View.hpp>
#include <GlobalMgr.h>
#include <GameMgr.h>

unsigned long long uFrameCount = 0;

int main()
{
    sf::RenderWindow window(sf::VideoMode(GameMgr::GAME_SIZE_X * 2, GameMgr::GAME_SIZE_Y * 2), "SFML works!");
    ImGui::SFML::Init(window);

    sf::Clock clock;
    clock.restart();

    GlobalMgr::Instance()->Init();

    sf::View view(sf::Vector2f((float)GameMgr::GAME_SIZE_X / 2.f, (float)GameMgr::GAME_SIZE_Y / 2.f),
        sf::Vector2f(GameMgr::GAME_SIZE_X, GameMgr::GAME_SIZE_Y));
    window.setView(view);

    //system("pause");

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

            if (event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::O)
            {
                GlobalMgr::Instance()->GetGameMgr().SpawnEnemy();
            }

            ImGui::SFML::ProcessEvent(window, event);
        }

        PROFILER_EVENT_END();

        PROFILER_EVENT_BEGIN(PROFILER_COLOR_RED, "Update");
        //ImGui::SFML::Update(window, imGuiTime);

        GlobalMgr::Instance()->Update(fDeltaTimeS);

        // sample
        //ImGui::ShowDemoWindow();

        PROFILER_EVENT_END();

        PROFILER_EVENT_BEGIN(PROFILER_COLOR_GREEN, "Draw");
        window.clear();

        GlobalMgr::Instance()->Draw(window);

        //ImGui::SFML::Render(window);
        window.display();

        PROFILER_EVENT_END();

        PROFILER_EVENT_END();
        ++uFrameCount;
    }

    ImGui::SFML::Shutdown();

    GlobalMgr::Instance()->Delete();
    GlobalMgr::DeleteInstance();

    return 0;
}