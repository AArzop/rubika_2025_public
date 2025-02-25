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

	PROFILER_EVENT_BEGIN(PROFILER_COLOR_BLUE, "Load Image");

	TextureData textureData;
	if (!textureData.Image.loadFromFile(texturePath.generic_string()))
	{
		TexturesMutex.lock();
		Textures.emplace(texturePath, std::move(TextureData()));
		TexturesMutex.unlock();
	}
	else
	{
		TexturesMutex.lock();
		Textures.emplace(texturePath, std::move(textureData));
		TexturesMutex.unlock();
	}
	PROFILER_EVENT_END();

	CallbacksNextFrameMutex.lock();
	CallbacksNextFrame.insert(texturePath);
	CallbacksNextFrameMutex.unlock();

	PROFILER_EVENT_END();
}

TextureMgr::sLoadCallback::sLoadCallback(const std::filesystem::path& texturePath, TextureMgr* pMgr) :
	Thread(std::bind(&TextureMgr::LoadTexture_Thread, pMgr, texturePath))
{}
