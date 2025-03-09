#pragma once

namespace sf
{
	class RenderWindow;
}

class Entity abstract
{
public:

	virtual void Init() = 0;
	virtual void Update(float deltaTime) = 0;
	virtual void Draw(sf::RenderWindow& window) = 0;
	virtual void Delete() = 0;
};