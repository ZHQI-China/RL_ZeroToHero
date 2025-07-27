#include "engine.h"

#include "util/logger.h"
#include "util/renderer.h"
#include "util/asset_store.h"

#include "core/scene/scene.h"
#include <Windows.h>
// 切换到英文输入法
static void changeToEnglish() {
	HWND hWnd = GetForegroundWindow();
	if (!hWnd) {
		MessageBox(NULL, "No active window found.", "Error", MB_OK | MB_ICONERROR);
		return;
	}

	HKL hklEnglish = LoadKeyboardLayout("00000409", KLF_ACTIVATE);
	if (!hklEnglish) {
		MessageBox(NULL, "Failed to load English keyboard layout.", "Error", MB_OK | MB_ICONERROR);
		return;
	}

	PostMessage(hWnd, WM_INPUTLANGCHANGEREQUEST, 0, (LPARAM)hklEnglish);
}

// 切换到中文输入法（简体中文）
static void changeToChinese() {

	HWND hWnd = GetForegroundWindow();
	if (!hWnd) {
		MessageBox(NULL, "No active window found.", "Error", MB_OK | MB_ICONERROR);
		return;
	}

    HKL hklChinese = LoadKeyboardLayout("00000804", KLF_ACTIVATE);
	if (!hklChinese) {
		MessageBox(NULL, "Failed to load English keyboard layout.", "Error", MB_OK | MB_ICONERROR);
		return;
	}

	PostMessage(hWnd, WM_INPUTLANGCHANGEREQUEST, 0, (LPARAM)hklChinese);
}

void Engine::run()
{
	
	// 模拟按键-切换中英文
    changeToEnglish();


    while(is_running_){
        auto start = SDL_GetTicksNS();
        handleEvents();
        update(dt_);
        render();
        auto end = SDL_GetTicksNS();
        auto elasped = end-start;
        if(elasped < frame_delay_ns){
            SDL_DelayNS(frame_delay_ns-elasped);
            dt_ = (float)(frame_delay_ns/1.e9);
        }else{
            dt_ = (float)(elasped/1.e9);
        }
        // SDL_Log("FPS: %f", 1.0/dt_);
    }

    changeToChinese();
}

void Engine::init(const std::string &title, int width, int height, int fps)
{
    screen_size_ = {width, height};
    title_ = title;

    // SDL3初始化
    if(!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)){
        GAME_ERROR("SDL3初始化失败: {}", SDL_GetError());
    }
    // SDL3_image 自动初始化
    // SDL3_Mixer 初始化
    if(Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG) != (MIX_INIT_MP3 | MIX_INIT_OGG)){
        GAME_ERROR("SDL_Mixer初始化失败: %s\n", SDL_GetError());
    }
    Mix_AllocateChannels(16); // 分配16个音频通道
    Mix_VolumeMusic(MIX_MAX_VOLUME/4); // 设置音乐音量
    Mix_Volume(-1, MIX_MAX_VOLUME/4); // 设置所有音频通道的音量

    // SDL3_ttf 初始化
    if(!TTF_Init()){
        GAME_ERROR("SDL_ttf初始化失败: %s\n", SDL_GetError());
    }

    // 创建窗口和渲染器
    SDL_CreateWindowAndRenderer(title.c_str(), width, height, SDL_WINDOW_RESIZABLE, &window_, &renderer_);
    if(!window_ || !renderer_){
        GAME_ERROR("窗口或渲染器创建失败: %s\n", SDL_GetError());
    }
    // 设置窗口逻辑分辨率
    SDL_SetRenderLogicalPresentation(renderer_, width, height,SDL_LOGICAL_PRESENTATION_LETTERBOX);

    if(fps<0){
        GAME_ERROR("FPS设置错误: %d<=0\n, 设置为60", fps);
        fps = 60;
    }else{
        fps_= fps;
    }
    frame_delay_ns = (Uint64)(1e9/fps_);

     
     AssetStore::init(renderer_); // 初始化资源管理器
     Renderer::init(renderer_); // 初始化渲染器

     ENGINE_INFO("引擎初始化完成");
}

void Engine::handleEvents()
{
    SDL_Event event;
    while(SDL_PollEvent(&event)){
        switch(event.type){
            case SDL_EVENT_QUIT:
                is_running_ = false;
                break;
            case SDL_EVENT_WINDOW_RESIZED:
                screen_size_ = {event.window.data1, event.window.data2};
                break;
            default:
                scenes_.at(current_scene_name_)->handleEvents(event);
                break;
        }
    }
}

void Engine::update(float dt)
{
    mouse_buttons_ = SDL_GetMouseState(&mouse_position_.x, &mouse_position_.y);
     scenes_.at(current_scene_name_)->update(dt);
}

void Engine::render()
{
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    SDL_RenderClear(renderer_);
    scenes_.at(current_scene_name_)->render();
    SDL_RenderPresent(renderer_);
}

void Engine::clean()
{
    // 清理并退出
    if( scenes_.at(current_scene_name_)){
        //scenes_.at(current_scene_name_)->clean();
        delete  scenes_.at(current_scene_name_);
        scenes_.at(current_scene_name_) = nullptr;
    }
    if (renderer_) {
		SDL_DestroyRenderer(renderer_);
		renderer_ = nullptr;
    }
    if (window_) {
		SDL_DestroyWindow(window_);
		window_ = nullptr;
    }

    Renderer::clean();
    AssetStore::clean();

    Mix_Quit();// 音频
    TTF_Quit(); // 字体

    SDL_Quit();
}

void Engine::addScene(Scene* new_scene)
{
    // 如果已经存在相同名字的场景
    if (scenes_.find(new_scene->name_) != scenes_.end()) {
        GAME_ERROR("添加场景失败，重复的场景命名：" + new_scene->name_);
        // todo: 删除new_scene？
        return;
    }
    //new_scene->init();
    scenes_.insert({ new_scene->name_,new_scene });
    ENGINE_INFO("添加场景：" + new_scene->name_);
}

void Engine::setCurrentScene(std::string name)
{
	// 如果场景不存在
	if (scenes_.find(name) == scenes_.end()) {
        GAME_ERROR("设置场景失败：" + name);
		return;
	}

    current_scene_name_ = name;
    scenes_.at(name)->init();

    // todo: 避免多次初始化

    ENGINE_INFO("设置当前场景为：" + name);
}



