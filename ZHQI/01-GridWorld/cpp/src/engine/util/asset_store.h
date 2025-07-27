#pragma once
#include <unordered_map>
#include <string>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "global/defs.h"

class AssetStore final
{
private:// 生命周期委托给 Engine 类管理
    friend class Engine;
public:
	static void init(SDL_Renderer* renderer) {
        renderer_ = renderer; 
        ttf_engine_ = TTF_CreateRendererTextEngine(renderer);
	}
    static void clean();
private://禁止实例化
	AssetStore() = delete;
	~AssetStore() = delete;
 
private:
    inline static SDL_Renderer* renderer_{};
    inline static  TTF_TextEngine* ttf_engine_{};  //文本引擎
    inline static std::unordered_map<std::string, SDL_Texture*> textures_;
    inline static std::unordered_map<std::string, Mix_Music*> musics_;
    inline static std::unordered_map<std::string, Mix_Chunk*> sounds_;
    inline static std::unordered_map<std::string, TTF_Font*> fonts_;
    
    // 4个载入函数
    static void loadTexture(const std::string& file_path);
    static void loadMusic(const std::string& file_path);
    static void loadSound(const std::string& file_path);
    static void loadFont(const std::string& file_path, int font_size);

public:
    // 4个读取函数
	static SDL_Texture* getTexture(const std::string& file_path);
	static Mix_Music* getMusic(const std::string& file_path);
	static Mix_Chunk* getSound(const std::string& file_path);
	static TTF_Font* getFont(const std::string& file_path, int font_size);

    static TTF_Text* createTTF_Text(const std::string& text, const std::string& font_path, int font_size = 16);
};