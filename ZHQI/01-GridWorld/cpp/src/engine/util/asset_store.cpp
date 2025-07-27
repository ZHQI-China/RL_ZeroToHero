#include "asset_store.h"

void AssetStore::loadTexture(const std::string &file_path)
{
    //SDL_Log("尝试 Loaded texture:%s",file_path.c_str());
    SDL_Texture* texture = IMG_LoadTexture(renderer_, file_path.c_str());
    if(texture == nullptr){
       SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"Failed to load texture:%s, %s",file_path.c_str(), SDL_GetError());
        return;
    }
    //SDL_Log("Loaded texture:%s",file_path.c_str());
    textures_.emplace(file_path, texture); //如果已经存在，则不会插入
}

void AssetStore::loadMusic(const std::string &file_path)
{
    Mix_Music* music = Mix_LoadMUS(file_path.c_str());
    if(music == nullptr){
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"Failed to load music:%s, %s",file_path.c_str(), SDL_GetError());
        return;
    }
    musics_.emplace(file_path, music);
}

void AssetStore::loadSound(const std::string &file_path)
{
    Mix_Chunk* sound = Mix_LoadWAV(file_path.c_str());
    if(sound == nullptr){
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"Failed to load sound:%s, %s",file_path.c_str(), SDL_GetError());
        return;
    }
    sounds_.emplace(file_path, sound);
}

void AssetStore::loadFont(const std::string &file_path, int font_size)
{
    TTF_Font* font = TTF_OpenFont(file_path.c_str(), (float)font_size);
    if(font == nullptr){
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"Failed to load font:%s, %s",file_path.c_str(), SDL_GetError());
        return;
    }
    fonts_.emplace(file_path+std::to_string(font_size), font);
}

SDL_Texture *AssetStore::getTexture(const std::string &file_path)
{
    auto iter = textures_.find(file_path);
    if(iter == textures_.end()){
        loadTexture(file_path);
        iter = textures_.find(file_path);
        if(iter == textures_.end()){
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"Failed to get texture:%s, %s",file_path.c_str(), SDL_GetError());
            return nullptr;
        }
    }
    return iter->second;
}

Mix_Music *AssetStore::getMusic(const std::string &file_path)
{
    auto iter = musics_.find(file_path);
    if(iter == musics_.end()){
        loadMusic(file_path);
        iter = musics_.find(file_path);
        if(iter == musics_.end()){
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"Failed to get music:%s, %s",file_path.c_str(), SDL_GetError());
            return nullptr;
        }
    }
    return iter->second;
}

Mix_Chunk *AssetStore::getSound(const std::string &file_path)
{
    auto iter = sounds_.find(file_path);
    if(iter == sounds_.end()){
        loadSound(file_path);
        iter = sounds_.find(file_path);
        if(iter == sounds_.end()){
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"Failed to get sound:%s, %s",file_path.c_str(), SDL_GetError());
            return nullptr;
        }
    }
    return iter->second;
}

TTF_Font *AssetStore::getFont(const std::string &file_path, int font_size)
{
    auto iter = fonts_.find(file_path+std::to_string(font_size));
    if(iter == fonts_.end()){
        loadFont(file_path, font_size);
        iter = fonts_.find(file_path+std::to_string(font_size));
        if(iter == fonts_.end()){
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"Failed to get font:%s, %s",(file_path+std::to_string(font_size)).c_str(), SDL_GetError());
            return nullptr;
        }
    }
    return iter->second;
}

void AssetStore::clean()
{
    for(auto &texture : textures_){
        SDL_DestroyTexture(texture.second);
    }
    textures_.clear();
    for(auto &music : musics_){
        Mix_FreeMusic(music.second);
    }
    musics_.clear();
    for(auto &sound : sounds_){
        Mix_FreeChunk(sound.second);
    }
    sounds_.clear();
    for(auto &font : fonts_){
        TTF_CloseFont(font.second);
    }
    fonts_.clear();
    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }
    if (ttf_engine_) {
		TTF_DestroyRendererTextEngine(ttf_engine_);
		ttf_engine_ = nullptr;
    }
}



TTF_Text* AssetStore::createTTF_Text(const std::string& text, const std::string& font_path, int font_size)
{
	auto font = AssetStore::getFont(font_path, font_size);
	return TTF_CreateText(ttf_engine_, font, text.c_str(), 0);
}