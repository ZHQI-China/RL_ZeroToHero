#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>
#include "math.h"
#include "global/defs.h"

class Renderer final {
	friend class Engine;
private://禁止实例化
	Renderer() = delete;
	~Renderer() = delete;
private:
	inline static SDL_Renderer* renderer_ = nullptr;
public:
	static void init(SDL_Renderer* renderer) { renderer_ = renderer;}
	static void clean();

	static SDL_Renderer* get() { return renderer_; };

	// 渲染函数
	static void renderTexture(const Texture& texture, const glm::vec2& pos, const glm::vec2& size, const glm::vec2& mask = glm::vec2(1.0f));
    static void renderTexture(SDL_Texture* texture, const glm::vec2& pos, const glm::vec2& size, float rotate_angle, SDL_FColor tint);
    
    static void renderFillCircle(const glm::vec2& position, const glm::vec2& size, float alpha);
	static void renderHBar(const glm::vec2& position, const glm::vec2& size, float percent, SDL_FColor color);




	static void drawGrid(const glm::vec2& top_left, const glm::vec2& bottom_right, float grid_width, SDL_FColor color);
	static void drawBoundary(const glm::vec2& top_left, const glm::vec2& bottom_right, int boundary_width, SDL_FColor color);
    static void drawArrow(const glm::vec2& start, const glm::vec2& end, vec2f arrow_scale = { 1.f ,1.f}, SDL_FColor line_color = { 1.f,1.f,1.f,1.f });
	static void drawLine(const glm::vec2& start, const glm::vec2& end, int line_width = 10, SDL_FColor line_color = {1.f,1.f,1.f,1.f});

	// 绘制旋转矩形：以中心为原点
	static void drawRectWithRotation(const SDL_FRect& rect, float rotation, SDL_FColor color = { 1.f, 1.f, 1.f, 0.8f });


    /**
  * @brief 使用 SDL3 的 SDL_RenderGeometry 渲染一个多边形。
  *
  * 注意:
  * - 此函数使用 SDL_RenderGeometry。
  * @param renderer SDL 渲染器。
  * @param points 多边形的顶点坐标列表 (std::vector<vec2f>)。
  * @param broad_thickness 边框的厚度。如果为 0 或负数，则不绘制边框。
  * @param boundary_color 边框的颜色 (SDL_FColor)。
  * @param fill_color 多边形内部的填充颜色 (SDL_FColor)。如果 alpha 值为 0，则不填充。
  */
    static void drawPoly(
        const std::vector<vec2f>& points,
        float broad_thickness,
        SDL_FColor boundary_color,
        SDL_FColor fill_color = {}) {

        if (points.size() < 2) {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "renderPoly: Not enough points to form a polygon (need at least 3, got %zu).", points.size());
            return;
        }
		if (points.size() < 3) {
            drawCircle(points[0], points[1][0], broad_thickness, boundary_color, fill_color);
			return;
		}
        // 1. 渲染填充 (如果 alpha > 0)
        if (fill_color.a > 0.0f) {
            std::vector<SDL_Vertex> fill_vertices;
            fill_vertices.reserve(points.size());
            for (const auto& p : points) {
                // SDL_Vertex: { position (SDL_FPoint), color (SDL_FColor), tex_coord (SDL_FPoint) }
                fill_vertices.push_back({ {p.x, p.y}, fill_color, {0.0f, 0.0f} });
            }

            // 为凸多边形创建三角扇索引
            // 对于凹多边形，这里产生的填充效果可能是错误的
            std::vector<int> fill_indices;
            if (points.size() >= 3) {
                fill_indices.reserve((points.size() - 2) * 3);
                for (size_t i = 1; i < points.size() - 1; ++i) {
                    fill_indices.push_back(0);
                    fill_indices.push_back(static_cast<int>(i));
                    fill_indices.push_back(static_cast<int>(i + 1));
                }
            }

            if (!fill_indices.empty()) {
                SDL_SetRenderDrawColorFloat(renderer_, fill_color.r, fill_color.g, fill_color.b, fill_color.a);
                if (!SDL_RenderGeometry(renderer_,
                    nullptr, // texture, 无纹理
                    fill_vertices.data(),
                    static_cast<int>(fill_vertices.size()),
                    fill_indices.data(),
                    static_cast<int>(fill_indices.size())) != 0) {
                    SDL_LogError(SDL_LOG_CATEGORY_RENDER, "renderPoly: Failed to render fill geometry: %s", SDL_GetError());
                }
            }
        }

        // 2. 渲染边框 (如果厚度 > 0 且 alpha > 0)
        if (broad_thickness > 0.0f && boundary_color.a > 0.0f) {
            std::vector<SDL_Vertex> boundary_vertices;
            std::vector<int> boundary_indices;
            float half_thickness = broad_thickness / 2.0f;

            for (size_t i = 0; i < points.size(); ++i) {
                const vec2f& p1 = points[i];
                const vec2f& p2 = points[(i + 1) % points.size()]; // 连接到下一个点，最后一个点连接到第一个

                float dx = p2.x - p1.x;
                float dy = p2.y - p1.y;
                float len = SDL_sqrtf(dx * dx + dy * dy);

                if (len == 0) continue; // 避免除以零

                // 法线向量 (向外)
                float nx = -dy / len;
                float ny = dx / len;

                int base_vertex_index = static_cast<int>(boundary_vertices.size());

                // 为当前线段创建四个顶点构成一个四边形
                // v0---v2 (p1_outer, p2_outer)
                // | \   |
                // |  \  |
                // |   \ |
                // v1---v3 (p1_inner, p2_inner)

                boundary_vertices.push_back({ {p1.x + nx * half_thickness, p1.y + ny * half_thickness}, boundary_color, {0.0f, 0.0f} }); // v0: p1_outer
                boundary_vertices.push_back({ {p1.x - nx * half_thickness, p1.y - ny * half_thickness}, boundary_color, {0.0f, 0.0f} }); // v1: p1_inner
                boundary_vertices.push_back({ {p2.x + nx * half_thickness, p2.y + ny * half_thickness}, boundary_color, {0.0f, 0.0f} }); // v2: p2_outer
                boundary_vertices.push_back({ {p2.x - nx * half_thickness, p2.y - ny * half_thickness}, boundary_color, {0.0f, 0.0f} }); // v3: p2_inner

                // 第一个三角形: v0, v1, v2
                boundary_indices.push_back(base_vertex_index + 0);
                boundary_indices.push_back(base_vertex_index + 1);
                boundary_indices.push_back(base_vertex_index + 2);

                // 第二个三角形: v1, v3, v2
                boundary_indices.push_back(base_vertex_index + 1);
                boundary_indices.push_back(base_vertex_index + 3);
                boundary_indices.push_back(base_vertex_index + 2);
            }

            if (!boundary_vertices.empty() && !boundary_indices.empty()) {
                SDL_SetRenderDrawColorFloat(renderer_, boundary_color.r, boundary_color.g, boundary_color.b, boundary_color.a);
                if (!SDL_RenderGeometry(renderer_,
                    nullptr, // texture
                    boundary_vertices.data(),
                    static_cast<int>(boundary_vertices.size()),
                    boundary_indices.data(),
                    static_cast<int>(boundary_indices.size())) != 0) {
                    SDL_LogError(SDL_LOG_CATEGORY_RENDER, "renderPoly: Failed to render boundary geometry: %s", SDL_GetError());
                }
            }
        }
    }

	static void drawCircleBoundary(const vec2f& center_pos, float radius, SDL_FColor color)
	{
		const float diameter = (radius * 2);

		float x = (radius - 1);
		float y = 0;
		float tx = 1;
		float ty = 1;
		float error = (tx - diameter);
        SDL_SetRenderDrawColorFloat(renderer_,color.r, color.g, color.b, color.a);
		while (x >= y)
		{
			//  Each of the following renders an octant of the circle
			SDL_RenderPoint(renderer_, center_pos.x + x, center_pos.y - y);
			SDL_RenderPoint(renderer_, center_pos.x + x, center_pos.y + y);
			SDL_RenderPoint(renderer_, center_pos.x - x, center_pos.y - y);
			SDL_RenderPoint(renderer_, center_pos.x - x, center_pos.y + y);
			SDL_RenderPoint(renderer_, center_pos.x + y, center_pos.y - x);
			SDL_RenderPoint(renderer_, center_pos.x + y, center_pos.y + x);
			SDL_RenderPoint(renderer_, center_pos.x - y, center_pos.y - x);
			SDL_RenderPoint(renderer_, center_pos.x - y, center_pos.y + x);

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
	}
    static void drawCircle(const vec2f& center_pos, float radius,
        float boundary_thickness = 2.f,
        SDL_FColor boundary_color = {1.f,1.f,1.f,0.6f},
        SDL_FColor fill_color = {}
        );
};