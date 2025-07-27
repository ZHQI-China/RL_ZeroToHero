#pragma once
#include <string>
#include <vector>
#include <SDL3/SDL.h>

// 纹理图片
struct Texture {
	SDL_Texture* texture{};
	SDL_FRect src_rect{};
	bool is_filp = false;
	float angle = 0.f;//默认锚点为中心
	Texture() = default;
	Texture(const std::string& file_path);
};

// Object 类型
enum class  ObjectType {
	None, World, Screen, Enemy
};

/// 锚点
enum class  Anchor
{
	TOP_LEFT, TOP_CENTER, TOP_RIGHT,
	CENTER_LEFT, CENTER, CENTER_RIGHT,
	BOTTOM_LEFT, BOTTOM_CENTER, BOTTOM_RIGHT,
	NONE,
};
inline static const std::vector<std::vector<int>> AnchorOffsets{
	{0,0},{1,0},{2,0},
	{0,1},{1,1},{2,1},
	{0,2},{1,2},{2,2},
};

// 方向
enum class  Direction {
	Down,Left,Up,Right
};

