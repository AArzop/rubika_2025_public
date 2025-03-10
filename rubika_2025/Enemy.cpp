#include "Enemy.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <GlobalMgr.h>
#include <GameMgr.h>


Enemy::Enemy() : Speed(100), Size(10, 10)
{}

void Enemy::Init()
{
	Hitbox.width = Size.x;
	Hitbox.height = Size.y;
	Hitbox.left = Position.x;
	Hitbox.top = Position.y;
}

void Enemy::Update(float deltaTime)
{
	Sprite.Update(deltaTime);

	switch (Direction)
	{
	case eDirection::Up:
		Position.y += Speed * deltaTime;
		break;
	case eDirection::Down:
		Position.y -= Speed * deltaTime;
		break;
	case eDirection::Left:
		Position.x -= Speed * deltaTime;
		break;
	case eDirection::Right:
		Position.x += Speed * deltaTime;
		break;
	case eDirection::None:
	case eDirection::Count:
	default:
		throw;
		break;
	}

	Hitbox.left = Position.x;
	Hitbox.top = Position.y;

	if (Position.x < -Size.x || Position.x > GameMgr::GAME_SIZE_X ||
		Position.y < -Size.y || Position.y > GameMgr::GAME_SIZE_Y)
	{
		GlobalMgr::Instance()->GetGameMgr().RequestEntityDeletion(this);
	}
}

void Enemy::Draw(sf::RenderWindow& window)
{
	sf::RenderStates states = sf::RenderStates::Default;
	states.transform.translate(Position);
	window.draw(Sprite.Get(), states);
}

void Enemy::Delete()
{
}

void Enemy::SetPosition(const sf::Vector2f& pos)
{
	Position = pos;
}

sf::Vector2f Enemy::GetPosition() const
{
	return Position;
}

void Enemy::SetSize(const sf::Vector2i& size)
{
	Size = size;
}

sf::Vector2i Enemy::GetSize() const
{
	return Size;
}

void Enemy::SetDirection(eDirection direction)
{
	Direction = direction;
}

void Enemy::SetTextureAndAnimation(const std::filesystem::path& path)
{
	Sprite.SetTexture(path);
	Sprite.SetAnimation("Default");
}

