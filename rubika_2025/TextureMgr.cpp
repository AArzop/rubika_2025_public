#include "TextureMgr.h"

#include <Profiler.h>
#include <rapidxml/rapidxml_utils.hpp>

#include <iostream>

#ifdef _DEBUG
#include <Imgui/imgui.h>
#endif

void TextureMgr::Init()
{
}

void TextureMgr::Update(float)
{
#if 0
	CallbacksNextFrameMutex.lock();
	for (const std::string& id : CallbacksNextFrame)
	{
		auto it = Requesting.find(id);
		if (it != Requesting.end())
		{
			const sLoadCallback& data = it->second;
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
			const sLoadCallback& data = it->second;
			TextureData* textureData = GetTextureData(id);
			if (textureData)
			{
				PROFILER_EVENT_BEGIN(PROFILER_COLOR_BLACK, "Load Texture from main");

				if (textureData->Image.getPixelsPtr())
				{
					textureData->Texture.loadFromImage(textureData->Image);
				}
				else
				{
					textureData->Texture = DefaultTexture;
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
			sLoadCallback& data = itEmplace.first->second;
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
		sLoadCallback& data = it->second;
		data.Callbacks.push_back(callback);
		data.UserData.push_back(userData);
	}
}

void TextureMgr::LoadTexture_Thread(std::filesystem::path texturePath)
{

	PROFILER_EVENT_BEGIN(PROFILER_COLOR_RED, "Requesting %s", texturePath.filename().c_str());

	if (!CheckTextureDependencies(texturePath) || !LoadTextureAndDependencies(texturePath))
	{
		TexturesMutex.lock();
		Textures.emplace(texturePath, std::move(TextureData()));
		TexturesMutex.unlock();
	}

	CallbacksNextFrameMutex.lock();
	CallbacksNextFrame.insert(texturePath);
	CallbacksNextFrameMutex.unlock();

	PROFILER_EVENT_END();
}

#ifdef _DEBUG

void TextureMgr::DrawDebug()
{

}

#endif

bool TextureMgr::CheckTextureDependencies(const std::filesystem::path& texturePath)
{
	if (!std::filesystem::exists(texturePath))
	{
		return false;
	}

	std::filesystem::path metadataPath = texturePath;
	metadataPath = metadataPath.replace_extension(".xml");
	if (!std::filesystem::exists(metadataPath))
	{
		return false;
	}

	return true;
}

bool TextureMgr::LoadTextureAndDependencies(const std::filesystem::path& texturePath)
{
	PROFILER_EVENT_BEGIN(PROFILER_COLOR_BLACK, "LoadTextureAndDependencies %s", texturePath.c_str());

	PROFILER_EVENT_BEGIN(PROFILER_COLOR_BLUE, "Load Image");

	TextureData textureData;
	if (!textureData.Image.loadFromFile(texturePath.generic_string()))
	{
		PROFILER_EVENT_END();
		PROFILER_EVENT_END();
		return false;
	}
	PROFILER_EVENT_END();

	PROFILER_EVENT_BEGIN(PROFILER_COLOR_GREEN, "Load Metadata");

	std::filesystem::path metadataPath = texturePath;
	metadataPath = metadataPath.replace_extension(".xml");
	if (!LoadTextureMetadata(metadataPath, textureData))
	{
		PROFILER_EVENT_END();
		PROFILER_EVENT_END();

		return false;
	}
	PROFILER_EVENT_END();

	PROFILER_EVENT_BEGIN(PROFILER_COLOR_RED, "Lock");
	textureData.Path = texturePath;
	TexturesMutex.lock();
	Textures.emplace(texturePath, std::move(textureData));
	TexturesMutex.unlock();
	PROFILER_EVENT_END();

	PROFILER_EVENT_END();
	return true;
}

bool TextureMgr::LoadTextureMetadata(const std::filesystem::path& path, TextureData& textureData)
{
	rapidxml::file<> metadataFile(path.generic_string().c_str());
	if (metadataFile.size() == 0)
	{
		std::cerr << "LoadTextureMetadata: Cannot open file " << path << std::endl;
		return false;
	}

	rapidxml::xml_document metadataXml;
	metadataXml.parse<0>(metadataFile.data());

	if (rapidxml::xml_node<>* node = metadataXml.first_node("Animations"))
	{
		if (!LoadAnimationMetadata(node, textureData))
		{
			return false;
		}
	}

	if (rapidxml::xml_node<>* node = metadataXml.first_node("Backgrounds"))
	{
		if (!LoadStaticTileMetadata(node, textureData))
		{
			return false;
		}
	}

	return true;
}

bool TextureMgr::LoadAnimationMetadata(rapidxml::xml_node<>* node, TextureData& textureData)
{
	if (!node)
	{
		return false;
	}

	rapidxml::xml_node<>* animationNode = node->first_node();
	while (animationNode)
	{
		rapidxml::xml_attribute<>* nameAttribute = animationNode->first_attribute("Name");
		if (nameAttribute)
		{
			auto p = textureData.AnimationsData.emplace(std::string(nameAttribute->value(), nameAttribute->value_size()), AnimationData());
			if (p.second)
			{
				AnimationData& data = p.first->second;
				if (rapidxml::xml_node<>* innerNode = animationNode->first_node("X"))
				{
					std::string value(innerNode->value(), innerNode->value_size());
					data.StartX = atoi(value.c_str());
				}

				if (rapidxml::xml_node<>* innerNode = animationNode->first_node("Y"))
				{
					std::string value(innerNode->value(), innerNode->value_size());
					data.StartY = atoi(value.c_str());
				}

				if (rapidxml::xml_node<>* innerNode = animationNode->first_node("SizeX"))
				{
					std::string value(innerNode->value(), innerNode->value_size());
					data.SizeX = atoi(value.c_str());
				}

				if (rapidxml::xml_node<>* innerNode = animationNode->first_node("SizeY"))
				{
					std::string value(innerNode->value(), innerNode->value_size());
					data.SizeY = atoi(value.c_str());
				}

				if (rapidxml::xml_node<>* innerNode = animationNode->first_node("OffsetX"))
				{
					std::string value(innerNode->value(), innerNode->value_size());
					data.OffsetX = atoi(value.c_str());
				}

				if (rapidxml::xml_node<>* innerNode = animationNode->first_node("OffsetY"))
				{
					std::string value(innerNode->value(), innerNode->value_size());
					data.OffsetY = atoi(value.c_str());
				}

				if (rapidxml::xml_node<>* innerNode = animationNode->first_node("SpriteNum"))
				{
					std::string value(innerNode->value(), innerNode->value_size());
					data.AnimationSpriteCount = atoi(value.c_str());
				}

				if (rapidxml::xml_node<>* innerNode = animationNode->first_node("SpritesOnLine"))
				{
					std::string value(innerNode->value(), innerNode->value_size());
					data.SpriteOnLine = atoi(value.c_str());
				}

				if (rapidxml::xml_node<>* innerNode = animationNode->first_node("Reverted"))
				{
					std::string value(innerNode->value(), innerNode->value_size());
					data.IsReverted = atoi(value.c_str());
				}

				if (rapidxml::xml_node<>* innerNode = animationNode->first_node("TimeBetweenAnimation"))
				{
					std::string value(innerNode->value(), innerNode->value_size());
					data.TimeBetweenAnimationInS = std::stof(value.c_str());
				}
			}
			else
			{
				std::cerr << "LoadAnimationMetadata: Cannot add animation " << nameAttribute->value() << ". Ignore it" << std::endl;
			}
		}
		else
		{
			std::cerr << "LoadAnimationMetadata: Find a animation node with no name. Ignore it" << std::endl;
		}

		animationNode = animationNode->next_sibling();
	}

	return true;
}

bool TextureMgr::LoadStaticTileMetadata(rapidxml::xml_node<>* node, TextureData& textureData)
{
	if (!node)
	{
		return false;
	}

	rapidxml::xml_node<>* tileNode = node->first_node();
	while (tileNode)
	{
		rapidxml::xml_attribute<>* nameAttribute = tileNode->first_attribute("Name");
		if (nameAttribute)
		{
			auto p = textureData.StaticTilesData.emplace(std::string(nameAttribute->value(), nameAttribute->value_size()), StaticTileData());
			if (p.second)
			{
				StaticTileData& data = p.first->second;
				if (rapidxml::xml_node<>* innerNode = tileNode->first_node("X"))
				{
					std::string value(innerNode->value(), innerNode->value_size());
					data.StartX = atoi(value.c_str());
				}

				if (rapidxml::xml_node<>* innerNode = tileNode->first_node("Y"))
				{
					std::string value(innerNode->value(), innerNode->value_size());
					data.StartY = atoi(value.c_str());
				}

				if (rapidxml::xml_node<>* innerNode = tileNode->first_node("SizeX"))
				{
					std::string value(innerNode->value(), innerNode->value_size());
					data.SizeX = atoi(value.c_str());
				}

				if (rapidxml::xml_node<>* innerNode = tileNode->first_node("SizeY"))
				{
					std::string value(innerNode->value(), innerNode->value_size());
					data.SizeY = atoi(value.c_str());
				}

				if (rapidxml::xml_node<>* innerNode = tileNode->first_node("RevertedX"))
				{
					std::string value(innerNode->value(), innerNode->value_size());
					data.IsRevertedX = atoi(value.c_str());
				}

				if (rapidxml::xml_node<>* innerNode = tileNode->first_node("RevertedY"))
				{
					std::string value(innerNode->value(), innerNode->value_size());
					data.IsRevertedY = atoi(value.c_str());
				}
			}
			else
			{
				std::cerr << "LoadStaticTileMetadata: Cannot add static Data " << nameAttribute->value() << ". Ignore it" << std::endl;
			}
		}
		else
		{
			std::cerr << "LoadStaticTileMetadata: Find a tile node with no name. Ignore it" << std::endl;
		}

		tileNode = tileNode->next_sibling();
	}

	return true;
}

TextureMgr::sLoadCallback::sLoadCallback(const std::filesystem::path& texturePath, TextureMgr* pMgr) :
	Thread(std::bind(&TextureMgr::LoadTexture_Thread, pMgr, texturePath))
{}

AnimationData::AnimationData() : StartX(0), StartY(0), SizeX(0), SizeY(0),
OffsetX(0), OffsetY(0), AnimationSpriteCount(0), SpriteOnLine(0), IsReverted(false)
{}

TextureData::TextureData() : Texture(), AnimationsData()
{}

TextureData::~TextureData()
{
	AnimationsData.clear();
}
