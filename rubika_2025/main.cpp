#include <SFML/Graphics.hpp>
#include <Imgui/imgui.h>
#include <Imgui/imgui-SFML.h>
#include "Profiler.h"

unsigned long long uFrameCount = 0;

int main()
{
    sf::RenderWindow window(sf::VideoMode(720, 480), "SFML works!");
    ImGui::SFML::Init(window);

    sf::Clock clock;
    clock.restart();

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

            ImGui::SFML::ProcessEvent(window, event);
        }

        PROFILER_EVENT_END();

        PROFILER_EVENT_BEGIN(PROFILER_COLOR_RED, "Update");
        ImGui::SFML::Update(window, imGuiTime);

        // sample
        ImGui::ShowDemoWindow();

        PROFILER_EVENT_END();

        PROFILER_EVENT_BEGIN(PROFILER_COLOR_GREEN, "Draw");
        window.clear();

        ImGui::SFML::Render(window);
        window.display();

        PROFILER_EVENT_END();

        PROFILER_EVENT_END();
        ++uFrameCount;
    }

    ImGui::SFML::Shutdown();

    return 0;
}