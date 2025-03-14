#pragma once

#include <Entity.h>
#include <Direction.h>
#include <Sprite.h>
#include <SFML/Graphics/Rect.hpp>


namespace sf
{
	class Texture;
}

class Enemy : public Entity
{
public:
	Enemy();

	virtual void Init() override;
	virtual void Update(float deltaTime) override;
	virtual void Draw(sf::RenderWindow& window) override;
	virtual void Delete() override;

	void SetPosition(const sf::Vector2f& pos);
	sf::Vector2f GetPosition() const;

	void SetSize(const sf::Vector2i& size);
	sf::Vector2i GetSize() const;

	void SetDirection(eDirection direcion);
	void SetTextureAndAnimation(const std::filesystem::path& path);

	const sf::FloatRect& GetHitbox() const { return Hitbox; }

private:

	sf::Vector2f Position;
	Sprite Sprite;
	sf::Vector2i Size;

	float Speed = 10.f;
	eDirection Direction;

	sf::FloatRect Hitbox;
};