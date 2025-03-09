#include "Sprite.h"

#include <TextureMgr.h>
#include <GlobalMgr.h>

void Sprite::Start()
{}

void Sprite::Update(float deltaTime)
{
	if (PlayAnimation)
	{
		CurrentAnimationTime += deltaTime;
		if (CurrentAnimationTime < CurrentAnimationData->TimeBetweenAnimationInS)
		{
			++CurrentAnimationNb;
			if (CurrentAnimationNb >= CurrentAnimationData->AnimationSpriteCount)
			{
				CurrentAnimationNb = 0;
			}
			CurrentAnimationTime = 0;
		}
	}

	unsigned line = CurrentAnimationNb / CurrentAnimationData->SpriteOnLine;
	unsigned column = CurrentAnimationNb % CurrentAnimationData->SpriteOnLine;

	sf::IntRect rect;

	if (!CurrentAnimationData->IsReverted)
	{
		rect.height = CurrentAnimationData->SizeY;
		rect.top = CurrentAnimationData->StartY + line * (CurrentAnimationData->OffsetY + CurrentAnimationData->SizeY);

		rect.width = CurrentAnimationData->SizeX;
		rect.left = CurrentAnimationData->StartX + column * (CurrentAnimationData->OffsetX + CurrentAnimationData->SizeX);
	}
	else
	{
		rect.height = CurrentAnimationData->SizeY;
		rect.top = CurrentAnimationData->StartY + line * (CurrentAnimationData->OffsetY + CurrentAnimationData->SizeY);

		rect.width = -CurrentAnimationData->SizeX;
		rect.left = CurrentAnimationData->StartX + column * (CurrentAnimationData->OffsetX + CurrentAnimationData->SizeX) + CurrentAnimationData->SizeX;
	}

	sfSprite.setTextureRect(rect);
}

void Sprite::Reset()
{
	CurrentAnimationTime = 0.f;
	CurrentAnimationNb = 0;
}

void Sprite::SetTexture(const std::filesystem::path& textureName)
{
	const TextureData* textureData = GlobalMgr::Instance()->GetTextureMgr().GetTextureData(textureName);
	if (textureData)
	{
		sfSprite.setTexture(textureData->Texture);
	}
	CurrentTexture = textureName;
}

void Sprite::SetAnimation(const std::string& animationName)
{
	const TextureData* textureData = GlobalMgr::Instance()->GetTextureMgr().GetTextureData(CurrentTexture);
	if (textureData)
	{
		const auto& it = textureData->AnimationsData.find(animationName);
		if (it != textureData->AnimationsData.end())
		{
			CurrentAnimationData = &it->second;
		}
		else
		{
			CurrentAnimationData = nullptr;
		}
	}

	CurrentAnimation = animationName;
}

void Sprite::EnableAnimation(bool enable)
{
	//PlayAnimation = enable;
}
