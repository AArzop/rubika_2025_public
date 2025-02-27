#include "Sprite.h"

#include <GlobalMgr.h>
#include <TextureMgr.h>

void Sprite::Init()
{
	CurrentAnimationData = nullptr;
}

void Sprite::Update(float deltaTime)
{
	if (!PlayAnimation)
	{
		return;
	}

	CurrentAnimationTime += deltaTime;
	if (CurrentAnimationTime > CurrentAnimationData->TimeBetweenAnimationInS)
	{
		CurrentAnimationTime = 0.f;
		++CurrentAnimationNb;
		if (CurrentAnimationNb > CurrentAnimationData->AnimationSpriteCount)
		{
			CurrentAnimationNb = 0;
		}
	}

	unsigned line = CurrentAnimationNb / CurrentAnimationData->SpriteOnLine;
	unsigned coluln = CurrentAnimationNb % CurrentAnimationData->SpriteOnLine;

	sf::IntRect rect;
	rect.width = CurrentAnimationData->SizeX;
	rect.height = CurrentAnimationData->SizeY;

	rect.top = CurrentAnimationData->StartY + line * (CurrentAnimationData->SizeY + CurrentAnimationData->OffsetY);
	rect.left = CurrentAnimationData->StartX + coluln * (CurrentAnimationData->SizeX + CurrentAnimationData->OffsetX);

	sfSprite.setTextureRect(rect);
}

void Sprite::Reset()
{
}

void Sprite::SetTexture(const std::filesystem::path& textureName)
{
	TextureMgr& textureMgr = GlobalMgr::Instance()->GetTextureMgr();
	const sf::Texture* texture = textureMgr.GetTexture(textureName);
	if (texture)
	{
		sfSprite.setTexture(*texture);
		CurrentTexture = textureName;
	}
}

void Sprite::SetAnimation(const std::string& animationName)
{
	TextureMgr& textureMgr = GlobalMgr::Instance()->GetTextureMgr();
	const TextureData* textureData = textureMgr.GetTextureData(CurrentTexture);
	if (!textureData)
	{
		return;
	}

	const auto& it = textureData->AnimationsData.find(animationName);
	if (it != textureData->AnimationsData.end())
	{
		CurrentAnimationData = &it->second;
	}
}

void Sprite::EnableAnimation(bool enable)
{
	PlayAnimation = enable;
}
