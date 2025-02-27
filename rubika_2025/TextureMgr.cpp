#include "TextureMgr.h"

#include <Profiler.h>
#include <rapidxml/rapidxml_utils.hpp>

#include <iostream>

void TextureMgr::Init()
{}

void TextureMgr::Update(float)
{
#if 0
	CallbacksNextFrameMutex.lock();
	for (const std::string& id : CallbacksNextFrame)
	{
		auto it = Requesting.find(id);
		if (it != Requesting.end())
		{
			const RequestData& data = it->second;
			const sf::Texture* texture = GetTexture(id);
			for (int i = 0; i < data.Callbacks.size(); ++i)
			{
				data.Callbacks[i](texture, data.UserData[i]);
			}
			Requesting.erase(it);
		}
	}
	CallbacksNextFrame.clear();
	CallbacksNextFrameMutex.unlock();
#else
	CallbacksNextFrameMutex.lock();
	std::set<std::filesystem::path> localCallkacks;
	std::swap(localCallkacks, CallbacksNextFrame);
	CallbacksNextFrameMutex.unlock();

	for (const std::filesystem::path& id : localCallkacks)
	{
		auto it = Requesting.find(id);
		if (it != Requesting.end())
		{
			const RequestData& data = it->second;
			TextureData* textureData = GetTextureData(id);
			if (textureData)
			{
				PROFILER_EVENT_BEGIN(PROFILER_COLOR_BLACK, "Load Texture from main");

				if (textureData->Image.getPixelsPtr())
				{
					textureData->Texture.loadFromImage(textureData->Image);
				}

				PROFILER_EVENT_END();
			}

			for (int i = 0; i < data.Callbacks.size(); ++i)
			{
				data.Callbacks[i](textureData, data.UserData[i]);
			}

			Requesting.erase(it);
		}
	}
#endif
}

void TextureMgr::Reset()
{
	TexturesMutex.lock();
	Textures.clear();
	TexturesMutex.unlock();
}

const TextureData* TextureMgr::GetTextureData(const std::filesystem::path& texturePath) const
{
	const TextureData* ret = nullptr;

	TexturesMutex.lock();
	const auto& it = Textures.find(texturePath);
	if (it != Textures.end())
	{
		ret = (&it->second);
	}
	TexturesMutex.unlock();

	return ret;
}

TextureData* TextureMgr::GetTextureData(const std::filesystem::path& texturePath)
{
	TextureData* ret = nullptr;

	TexturesMutex.lock();
	auto it = Textures.find(texturePath);
	if (it != Textures.end())
	{
		ret = (&it->second);
	}
	TexturesMutex.unlock();

	return ret;
}

const sf::Texture* TextureMgr::GetTexture(const std::filesystem::path& path) const
{
	const sf::Texture* ret = nullptr;

	TexturesMutex.lock();
	const auto& it = Textures.find(path);
	if (it != Textures.end())
	{
		ret = (&it->second.Texture);
	}
	TexturesMutex.unlock();

	return ret;
}

void TextureMgr::RequestLoadTexture(const std::filesystem::path& texturePath, LoadCallback callback, void* userData)
{
	const TextureData* texture = GetTextureData(texturePath);
	if (texture)
	{
		callback(texture, userData);
		return;
	}

	auto it = Requesting.find(texturePath);
	if (it == Requesting.end())
	{
		auto itEmplace = Requesting.emplace(std::piecewise_construct,
			std::forward_as_tuple(texturePath),
			std::forward_as_tuple(texturePath, this));
		if (itEmplace.second)
		{
			RequestData& data = itEmplace.first->second;
			data.Callbacks.push_back(callback);
			data.UserData.push_back(userData);
			data.Thread.launch();
		}
		else
		{
			callback(nullptr, userData);
		}
	}
	else
	{
		RequestData& data = it->second;
		data.Callbacks.push_back(callback);
		data.UserData.push_back(userData);
	}
}

void TextureMgr::LoadTexture_Thread(std::filesystem::path texturePath)
{
	PROFILER_EVENT_BEGIN(PROFILER_COLOR_RED, "Requesting %s", texturePath.filename().c_str());

	PROFILER_EVENT_BEGIN(PROFILER_COLOR_BLUE, "Load Image");

	if (!LoadTextureAndDependencies(texturePath))
	{
		TexturesMutex.lock();
		Textures.emplace(texturePath, std::move(TextureData()));
		TexturesMutex.unlock();
	}

	PROFILER_EVENT_END();

	CallbacksNextFrameMutex.lock();
	CallbacksNextFrame.insert(texturePath);
	CallbacksNextFrameMutex.unlock();

	PROFILER_EVENT_END();
}

bool TextureMgr::CheckTextureDependencies(const std::filesystem::path& texturePath)
{
	if (!std::filesystem::exists(texturePath))
	{
		return false;
	}

	std::filesystem::path metadataPath = texturePath;
	metadataPath.replace_extension(".xml");

	if (!std::filesystem::exists(metadataPath))
	{
		return false;
	}

	return true;
}

bool TextureMgr::LoadTextureAndDependencies(const std::filesystem::path& texturePath)
{
	if (!CheckTextureDependencies(texturePath))
	{
		return false;
	}

	TextureData textureData;
	textureData.Path = texturePath;
	if (!textureData.Image.loadFromFile(texturePath.generic_string()))
	{
		return false;
	}

	std::filesystem::path metadataPath = texturePath;
	metadataPath.replace_extension(".xml");

	if (!LoadTextureMetadata(metadataPath, textureData))
	{
		return false;
	}

	TexturesMutex.lock();
	Textures.emplace(texturePath, std::move(textureData));
	TexturesMutex.unlock();
	return true;
}

bool TextureMgr::LoadTextureMetadata(const std::filesystem::path& path, TextureData& textureData)
{
	rapidxml::file<> file(path.generic_string().c_str());
	rapidxml::xml_document<> doc;
	doc.parse<0>(file.data());

	rapidxml::xml_node<>* rootNode = doc.first_node();
	if (!rootNode)
	{
		return false;
	}

	bool b = LoadAnimationMetadata(rootNode, textureData);
	return b;
}

bool TextureMgr::LoadAnimationMetadata(rapidxml::xml_node<>* node, TextureData& textureData)
{
	if (!node)
	{
		return false;
	}

	rapidxml::xml_node<>* animationNode = node->first_node("Animation");
	while (animationNode)
	{
		rapidxml::xml_attribute<>* nameAttr = animationNode->first_attribute("Name");
		if (!nameAttr || nameAttr->value_size() == 0)
		{
			animationNode = animationNode->next_sibling("Animation");
			continue;
		}

		std::string name = nameAttr->value();

		AnimationData animData;

		{
			rapidxml::xml_node<>* node = animationNode->first_node("X");
			if (node)
			{
				animData.StartX = std::atoi(node->value());
			}
		}

		{
			rapidxml::xml_node<>* node = animationNode->first_node("Y");
			if (node)
			{
				animData.StartY = std::atoi(node->value());
			}
		}

		{
			rapidxml::xml_node<>* node = animationNode->first_node("SizeX");
			if (node)
			{
				animData.SizeX = std::atoi(node->value());
			}
		}

		{
			rapidxml::xml_node<>* node = animationNode->first_node("SizeY");
			if (node)
			{
				animData.SizeY = std::atoi(node->value());
			}
		}

		{
			rapidxml::xml_node<>* node = animationNode->first_node("OffsetX");
			if (node)
			{
				animData.OffsetX = std::atoi(node->value());
			}
		}

		{
			rapidxml::xml_node<>* node = animationNode->first_node("OffsetY");
			if (node)
			{
				animData.OffsetY = std::atoi(node->value());
			}
		}

		{
			rapidxml::xml_node<>* node = animationNode->first_node("SpriteNum");
			if (node)
			{
				animData.AnimationSpriteCount = std::atoi(node->value());
			}
		}

		{
			rapidxml::xml_node<>* node = animationNode->first_node("SpritesOnLine");
			if (node)
			{
				animData.SpriteOnLine = std::atoi(node->value());
			}
		}

		{
			rapidxml::xml_node<>* node = animationNode->first_node("Reverted");
			if (node)
			{
				animData.IsReverted = std::atoi(node->value());
			}
		}

		{
			rapidxml::xml_node<>* node = animationNode->first_node("TimeBetweenAnimation");
			if (node)
			{
				animData.TimeBetweenAnimationInS = std::atof(node->value());
			}
		}

		textureData.AnimationsData.emplace(name, animData);
		animationNode = animationNode->next_sibling("Animation");
	}

	return true;
}

TextureMgr::RequestData::RequestData(const std::filesystem::path& texturePath, TextureMgr* pMgr) :
	Thread(std::bind(&TextureMgr::LoadTexture_Thread, pMgr, texturePath))
{}

AnimationData::AnimationData() : StartX(0), StartY(0), SizeX(0), SizeY(0),
OffsetX(0), OffsetY(0), AnimationSpriteCount(0), SpriteOnLine(0), IsReverted(false)
{}
