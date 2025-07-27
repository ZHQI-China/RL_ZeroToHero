#include "renderer.h"
#include "asset_store.h"

void Renderer::clean() {
	if (!renderer_) return;
	SDL_DestroyRenderer(renderer_);
	renderer_ = nullptr;
}

void Renderer::renderTexture(const Texture& texture, const glm::vec2& pos, const glm::vec2& size, const glm::vec2& mask)
{
	SDL_FRect src_rect = {
		texture.src_rect.x,
		texture.src_rect.y + texture.src_rect.h * (1 - mask.y),
		texture.src_rect.w * mask.x,
		texture.src_rect.h * mask.y
	};
	SDL_FRect dst_rect = {
		pos.x,
		pos.y + size.y * (1 - mask.y),
		size.x * mask.x,
		size.y * mask.y
	};

	SDL_RenderTextureRotated(renderer_, texture.texture,
		&src_rect, &dst_rect, texture.angle,
		nullptr, texture.is_filp ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE
	);

	//debug
	SDL_FRect rect{ pos.x,pos.y,size.x,size.y };
	SDL_RenderRect(renderer_, &rect);
}

void Renderer::renderTexture(SDL_Texture* texture, const glm::vec2& pos, const glm::vec2& size, float rotate_angle, SDL_FColor tint)
{
	SDL_FRect src_rect = {
		0,
		0,
		texture->w,
		texture->h
	};
	SDL_FRect dst_rect = {
		pos.x,
		pos.y,
		size.x,
		size.y
	};
	SDL_SetTextureColorModFloat(texture, tint.r, tint.g, tint.b);
	SDL_RenderTextureRotated(renderer_, texture, &src_rect, &dst_rect, rotate_angle, nullptr, SDL_FLIP_NONE);
	//SDL_RenderTexture(renderer_, texture, &src_rect, &dst_rect);
	//SDL_SetTextureAlphaModFloat()
}

void Renderer::renderFillCircle(const glm::vec2& position, const glm::vec2& size, float alpha)
{
	auto texture = AssetStore::getTexture("assets/UI/circle.png");
	SDL_FRect dst_rect = {
		position.x, position.y, size.x, size.y
	};
	SDL_SetTextureAlphaModFloat(texture, alpha);
	SDL_RenderTexture(renderer_, texture, NULL, &dst_rect);
}

//void Renderer::renderFillCircle(const glm::vec2& center_pos, float radius, float alpha)
//{
//	// 获取圆形纹理
//	auto texture = AssetStore::getTexture("src/sandbox/assets/UI/circle.png");
//
//	// 计算目标矩形：以 center_pos 为中心，宽高为 2 * radius
//	SDL_FRect dst_rect = {
//		center_pos.x - radius,      // 左上角 x 坐标
//		center_pos.y - radius,      // 左上角 y 坐标
//		2.0f * radius,              // 宽度
//		2.0f * radius               // 高度
//	};
//
//	// 设置纹理的透明度
//	SDL_SetTextureAlphaModFloat(texture, alpha);
//
//	// 渲染纹理到目标矩形
//	SDL_RenderTexture(renderer_, texture, NULL, &dst_rect);
//}

void Renderer::renderHBar(const glm::vec2& position, const glm::vec2& size, float percent, SDL_FColor fcolor)
{
	SDL_SetRenderDrawColorFloat(renderer_, fcolor.r, fcolor.g, fcolor.b, fcolor.a);
	SDL_FRect boundary_rect = {
		position.x,
		position.y,
		size.x,
		size.y
	};
	SDL_FRect fill_rect = {
		position.x,
		position.y,
		size.x * percent,
		size.y
	};
	SDL_RenderRect(renderer_, &boundary_rect);
	SDL_RenderFillRect(renderer_, &fill_rect);
	SDL_SetRenderDrawColorFloat(renderer_, 0, 0, 0, 1);
}

void Renderer::drawGrid(const glm::vec2& top_left, const glm::vec2& bottom_right, float grid_width, SDL_FColor color)
{
	SDL_SetRenderDrawColorFloat(renderer_, color.r, color.g, color.b, color.a);
	for (float x = top_left.x; x <= bottom_right.x; x += grid_width) {
		SDL_RenderLine(renderer_, x, top_left.y, x, bottom_right.y);
	}
	for (float y = top_left.y; y <= bottom_right.y; y += grid_width) {
		SDL_RenderLine(renderer_, top_left.x, y, bottom_right.x, y);
	}
	SDL_SetRenderDrawColorFloat(renderer_, 1.0f, 1.0f, 1.0f, 1.0f);
}

void Renderer::drawBoundary(const glm::vec2& top_left, const glm::vec2& bottom_right, int boundary_width, SDL_FColor color)
{
	SDL_SetRenderDrawColorFloat(renderer_, color.r, color.g, color.b, color.a);
	// 矩形叠加
	for (int i = 0;i < boundary_width;i++) {
		SDL_FRect rect{
			top_left.x - i,
			top_left.y - i,
			bottom_right.x - top_left.x + i * 2,
			bottom_right.y - top_left.y + i * 2
		};
		SDL_RenderRect(renderer_, &rect);
	}
	SDL_SetRenderDrawColorFloat(renderer_, 1.0f, 1.0f, 1.0f, 1.0f);
}

void Renderer::drawArrow(const glm::vec2& start, const glm::vec2& end, vec2f arrow_scale, SDL_FColor line_color)
{
	// 计算方向向量
	glm::vec2 dir = end - start;
	float length = glm::length(dir);
	if (length == 0.0f)
		return;

	// 计算单位方向向量
	glm::vec2 unit_dir = dir / length;

	// 计算箭头头参数
	float head_length = 15* arrow_scale.x;
	float head_width = 10* arrow_scale.y;

	// 计算箭头头的各个点
	glm::vec2 p = end - unit_dir * head_length;
	glm::vec2 perpendicular(-unit_dir.y, unit_dir.x);
	glm::vec2 p1 = p + perpendicular * (head_width / 2.0f);
	glm::vec2 p2 = p - perpendicular * (head_width / 2.0f);

	// 绘制主线条
	drawLine(start, end);

	// 绘制箭头头的三个边
	drawLine(end, p1);
	drawLine(end, p2);
	drawLine(p1, p2);
}

void Renderer::drawLine(const glm::vec2& start, const glm::vec2& end, int line_width, SDL_FColor line_color)
{
	SDL_SetRenderDrawColorFloat(renderer_, line_color.r, line_color.g, line_color.b, line_color.a);
	SDL_RenderLine(renderer_, start.x, start.y, end.x, end.y);
	SDL_SetRenderDrawColorFloat(renderer_, 1.f,1.f,1.f,1.f);
}

void Renderer::drawRectWithRotation(const SDL_FRect& rect, float rotation, SDL_FColor color)
{
	auto texture = AssetStore::getTexture(R"(assets\sprite\rect.png)");

	

	SDL_SetTextureColorModFloat(texture, color.r, color.g, color.b);
	SDL_SetTextureAlphaModFloat(texture, color.a);
	
	SDL_FRect src_rect = { 0,0,(float)texture->w,(float)texture->h };
	SDL_RenderTextureRotated(renderer_, texture, &src_rect, &rect, rotation, nullptr, SDL_FlipMode::SDL_FLIP_NONE);

	//debug
	//SDL_FRect rect{ pos.x,pos.y,size.x,size.y };
	//SDL_RenderRect(renderer_, &rect);
}

void Renderer::drawCircle(const vec2f& center_pos, float radius,
	float boundary_thickness,
	SDL_FColor boundary_color ,
	SDL_FColor fill_color
	) {
	


	//do { drawCircleBoundary(center_pos, radius, fill_color); } while (radius-- > 0);
	/*if (fill_color.a>0.f) {
		auto texture = AssetStore::getTexture("assets/UI/circle.png");
		SDL_FRect dst_rect = {
			center_pos.x - radius, center_pos.y - radius, 2 * radius, 2 * radius
		};
		SDL_SetRenderDrawColorFloat(renderer_, fill_color.r, fill_color.g, fill_color.b, fill_color.a);
		SDL_SetTextureColorModFloat(texture, fill_color.r, fill_color.g, fill_color.b);
		SDL_SetTextureAlphaModFloat(texture, fill_color.a);
		SDL_RenderTexture(renderer_, texture, NULL, &dst_rect);
	}*/
	const float diameter = (radius * 2)+1;

	float x = (radius - 1);
	float y = 0;
	float tx = 1;
	float ty = 1;
	float error = (tx - diameter);
	std::vector<vec2f> points;
	while (x >= y)
	{
		//  Each of the following renders an octant of the circle
		points.emplace_back(vec2f{ center_pos.x + x, center_pos.y - y });
		points.emplace_back(vec2f{ center_pos.x + x, center_pos.y + y });
		points.emplace_back(vec2f{ center_pos.x - x, center_pos.y - y });
		points.emplace_back(vec2f{ center_pos.x - x, center_pos.y + y });
		points.emplace_back(vec2f{ center_pos.x + y, center_pos.y - x });
		points.emplace_back(vec2f{ center_pos.x + y, center_pos.y + x });
		points.emplace_back(vec2f{ center_pos.x - y, center_pos.y - x });
		points.emplace_back(vec2f{ center_pos.x - y, center_pos.y + x });
		if (error <= 0)
		{
			++y;
			error += ty;
			ty += 2;
		}

		if (error > 0)
		{
			--x;
			tx += 2;
			error += (tx - diameter);
		}
	}
	
	drawPoly(points, 0.f, {}, fill_color);
	do { drawCircleBoundary(center_pos, radius--, boundary_color); } while (boundary_thickness-- > 0);
}
