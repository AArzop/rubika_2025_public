#include "Player.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <GlobalMgr.h>
#include <GameMgr.h>


Player::Player() : Speed(100), Size(20, 15)
{
}

void Player::Init()
{
	Hitbox.width = Size.x;
	Hitbox.height = Size.y;
	Hitbox.left = Position.x;
	Hitbox.top = Position.y;
}

void Player::Update(float deltaTime)
{
	eDirection dirX, dirY = eDirection::None;
	HandleInput(dirX, dirY);

	if (dirX != eDirection::None || dirY != eDirection::None)
	{
		HandleMove(deltaTime, dirX, dirY);
		Sprite.EnableAnimation(true);

		if (dirX == eDirection::Left)
		{
			if (Direction != dirX)
			{
				Sprite.SetAnimation("Body_Left");
				Direction = dirX;
			}
		}
		else if (dirX == eDirection::Right)
		{
			if (Direction != dirX)
			{
				Sprite.SetAnimation("Body_Right");
				Direction = dirX;
			}
		}
		else if (dirY == eDirection::Up)
		{
			if (Direction != dirY)
			{
				Sprite.SetAnimation("Body_Vertical");
				Direction = dirY;
			}
		}
		else if (dirY == eDirection::Down)
		{
			if (Direction != dirY)
			{
				Sprite.SetAnimation("Body_Vertical");
				Direction = dirY;
			}
		}
	}
	else
	{
		Sprite.EnableAnimation(false);
	}

	Sprite.Update(deltaTime);

	Hitbox.left = Position.x;
	Hitbox.top = Position.y;
}

void Player::Draw(sf::RenderWindow& window)
{
	sf::RenderStates states = sf::RenderStates::Default;
	states.transform.translate(Position);
	window.draw(Sprite.Get(), states);
}

void Player::Delete()
{
}

void Player::SetPosition(const sf::Vector2f& pos)
{
	Position = pos;
}

sf::Vector2f Player::GetPosition() const
{
	return Position;
}

void Player::SetSize(const sf::Vector2i& size)
{
	//Size = size;
}

sf::Vector2i Player::GetSize() const
{
	return Size;
}

void Player::SetDirection(eDirection direction)
{
	Direction = direction;
}

void Player::SetTextureAndAnimation(const std::filesystem::path& path)
{
	Sprite.SetTexture(path);
	Sprite.SetAnimation("Body_Left");
	SetDirection(eDirection::Left);
}

void Player::HandleInput(eDirection& dirX, eDirection& dirY) const
{
	dirX = eDirection::None;
	dirY = eDirection::None;

	bool bLeft = sf::Keyboard::isKeyPressed(sf::Keyboard::Q);
	bool bRight = sf::Keyboard::isKeyPressed(sf::Keyboard::D);
	bool bUp = sf::Keyboard::isKeyPressed(sf::Keyboard::Z);
	bool bDown = sf::Keyboard::isKeyPressed(sf::Keyboard::S);

	if (bLeft && !bRight)
	{
		dirX = eDirection::Left;
	}
	else if (!bLeft && bRight)
	{
		dirX = eDirection::Right;
	}

	if (bUp && !bDown)
	{
		dirY = eDirection::Up;
	}
	else if (!bUp && bDown)
	{
		dirY = eDirection::Down;
	}
}

void Player::HandleMove(float deltatime, const eDirection& dirX, const eDirection& dirY)
{
	if (dirX != eDirection::None)
	{
		if (dirX == eDirection::Left)
		{
			Position.x -= deltatime * Speed;
		}
		else
		{
			Position.x += deltatime * Speed;
		}

		Position.x = std::clamp(Position.x, 0.f, float(GameMgr::GAME_SIZE_X - Size.x));
	}

	if (dirY != eDirection::None)
	{
		if (dirY == eDirection::Down)
		{
			Position.y += deltatime * Speed;
		}
		else
		{
			Position.y -= deltatime * Speed;
		}

		Position.y = std::clamp(Position.y, 0.f, float(GameMgr::GAME_SIZE_Y - Size.y));
	}
}

