#pragma once

#include <SFML/Graphics/Sprite.hpp>
#include <filesystem>
#include <string>

class AnimationData;

class Sprite
{
public:
	void Start();
	void Update(float deltaTime);
	void Reset();

	void SetTexture(const std::filesystem::path& textureName);
	void SetAnimation(const std::string& animationName);
	void EnableAnimation(bool enable);

	sf::Sprite& Get() { return sfSprite; }

protected:

	sf::Sprite sfSprite;
	const AnimationData* CurrentAnimationData;

	std::filesystem::path CurrentTexture;
	std::string CurrentAnimation;

	unsigned CurrentAnimationNb = 0;
	float CurrentAnimationTime = 0.f;

	bool PlayAnimation = true;
};