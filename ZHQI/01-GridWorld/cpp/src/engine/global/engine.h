#pragma once
#include <string>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <glm/glm.hpp>
#include "global/defs.h"

class Engine{
private:
    glm::vec2 screen_size_{0,0};        // 屏幕尺寸
    std::string title_{""};             // 窗口标题
    SDL_Window* window_{nullptr};       // SDL窗口

    SDL_Renderer* renderer_{nullptr};   // SDL渲染器

    bool is_running_{true};             // 是否运行
    Uint64 fps_ = 60;                   // 帧率  
    Uint64 frame_delay_ns = 0;          // 帧延时
    float dt_ = 0.f;                    // 帧间隔
    
    glm::vec2 mouse_position_ = glm::vec2(0);//鼠标位置
    SDL_MouseButtonFlags mouse_buttons_ = 0;//鼠标按键信息

    std::unordered_map<std::string, class Scene*> scenes_;
    std::string current_scene_name_="";    // 当前场景

private:
    Engine(){}
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&)=delete;
public:
    static Engine& getInstance(){
        static Engine instance;
        return instance;
    }

    void init(const std::string& title, int width, int height, int fps = 60);
    void run();
    void handleEvents();
    void update(float dt);
    void render();
    void clean();

    glm::vec2 getScreenSize() const{ return screen_size_;}
    void addScene(class Scene* new_scene);
    class Scene* getCurrentScene() const{return scenes_.at(current_scene_name_);}
    void setCurrentScene(std::string name);

    glm::vec2 getMousePosition() const { return mouse_position_; } // 获取鼠标位置
    SDL_MouseButtonFlags getMouseButtons() const { return mouse_buttons_; } // 获取鼠标按键信息

public:
};