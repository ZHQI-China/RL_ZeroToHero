#pragma once
#include <SDL3/SDL.h>
#include <engine/util/math.h>
#include <engine/util/renderer.h>

struct AABB {
	vec2f min;
	vec2f max;
	AABB() {}
	AABB(vec2f min, vec2f max) { this->min = min;this->max = max; }
	bool intersects(const AABB& other) const {
		return !(
			max.x < other.min.x ||
			min.x > other.max.x ||
			max.y < other.min.y ||
			min.y > other.max.y
			);
	}

	bool contains(const vec2f& point) const {
		return point.x >= min.x && point.x <= max.x &&
			point.y >= min.y && point.y <= max.y;
	}
};

class Shape {
public:
	float rotation=0.f;//旋转角度0~	360: 顺时针
	float boundary_thickness = 2.f;
	SDL_FColor boundary_color = {1.f,1.f,1.f,1.f};
	SDL_FColor fill_color = {0.1f,0.1f,0.1f,1.0f};
public:
	virtual vec2f support(const vec2f& direction, vec2f offset = {}) const = 0;//获得给定方向上最远的点
	virtual void setRotation(float angle) { rotation = angle;}
	virtual std::vector<vec2f> getPointsRotated(vec2f offset = {}) const = 0;
	virtual std::vector<vec2f> getPoints(vec2f offset = {}) const = 0;
	virtual AABB getAABB(vec2f offset = {}) const = 0;
	virtual void draw(vec2f offset = {}) const = 0;
};

// 圆形
class Circle : public Shape {
public:
	vec2f center;
	float radius;

	Circle(vec2f center, float radius) : center(center), radius(radius) {}

	vec2f support(const vec2f& direction, vec2f offset = {}) const override {
		float len = std::sqrt(Dot(direction, direction));
		if (len == 0.0f) return center;
		return offset + center + direction * (radius / len);
	}
	// points[0] === center_pos, points[1][0]===radius
	std::vector<vec2f> getPoints(vec2f offset = {}) const override { return{ center + offset,{radius,0} }; }
	// points[0] === center_pos, points[1][0]===radius
	std::vector<vec2f> getPointsRotated(vec2f offset = {}) const override {
		return{ center + offset,{radius,0} };
	}
	AABB getAABB(vec2f offset = {}) const override {
		AABB aabb{};
		float halfSize = radius;
		aabb.min = vec2f(offset + center + vec2f(-halfSize, -halfSize)); // 左下
		aabb.max = vec2f(offset + center + vec2f(halfSize, halfSize));   // 右上
		return aabb;
	}
	void draw(vec2f offset = {}) const override {
		Renderer::drawCircle(center + offset, radius, boundary_thickness, boundary_color, fill_color);
	}
};

// 凸多边形：三角形、矩形、5、6...
class Polygon : public Shape {
public:
	std::vector<vec2f> points_;
	Polygon(const SDL_FRect& rect, float rotation = 0.f) {
		points_.reserve(4);
		points_.emplace_back(vec2f{rect.x,rect.y});
		points_.emplace_back(vec2f{rect.x+rect.w,rect.y});
		points_.emplace_back(vec2f{rect.x+rect.w,rect.y+rect.h});
		points_.emplace_back(vec2f{rect.x,rect.y+rect.h});
		this->rotation = rotation;
	}
	Polygon(const std::vector<vec2f>& points, float rotation = 0.f) : points_(points) {
		this->rotation = rotation;
		// todo: 凸性检查
	}

	vec2f support(const vec2f& direction, vec2f offset = {}) const override {
		float maxDot = -std::numeric_limits<float>::max();
		vec2f supportPoint{};

		// 计算几何中心
		vec2f center(0.0f, 0.0f);
		for (const auto& v : points_) {
			center += v;
		}
		if (!points_.empty()) {
			center /= static_cast<float>(points_.size());
		}

		// 预计算旋转角度的三角函数
		float cosA = cos(rotation * PI / 180.0f);
		float sinA = sin(rotation * PI / 180.0f);

		for (const auto& p : points_) {
			// 平移、旋转、再平移
			vec2f translated = p - center;
			vec2f rotated;
			rotated.x = translated.x * cosA - translated.y * sinA;
			rotated.y = translated.x * sinA + translated.y * cosA;
			vec2f rotatedPoint = rotated + center;

			float dot = Dot(rotatedPoint, direction);
			if (dot > maxDot) {
				maxDot = dot;
				supportPoint = rotatedPoint;
			}
		}

		return offset + supportPoint;
	}
	std::vector<vec2f> getPoints(vec2f offset = {}) const override { 
		if (offset.x==0.f && offset.y==0.f) return points_; 
		std::vector<vec2f> res = points_;
		for (auto& p : res) p += offset;
		return res;
	};

	std::vector<vec2f> getPointsRotated(vec2f offset = {}) const override {
		std::vector<vec2f> result;
		result.reserve(points_.size());

		// 1. 计算几何中心
		vec2f center(0.0f, 0.0f);
		for (const auto& v : points_) {
			center += v;
		}
		if (!points_.empty()) {
			center /= static_cast<float>(points_.size());
		}

		// 2. 预计算旋转角度的三角函数
		float cosA = cos(rotation * PI / 180.0f);
		float sinA = sin(rotation * PI / 180.0f);

		// 3. 对每个顶点进行旋转
		for (const auto& v : points_) {
			// 平移到原点
			vec2f translated = v - center;

			// 应用旋转
			vec2f rotated;
			rotated.x = translated.x * cosA - translated.y * sinA;
			rotated.y = translated.x * sinA + translated.y * cosA;

			// 平移回原来的位置
			vec2f finalPoint = rotated + center;
			result.push_back(finalPoint+offset);
		}

		return result;
	}

	AABB getAABB(vec2f offset = {}) const override {
		std::vector<vec2f> rotatedPoints = getPointsRotated();
		if (rotatedPoints.empty()) return {};

		float minX = rotatedPoints[0].x;
		float maxX = rotatedPoints[0].x;
		float minY = rotatedPoints[0].y;
		float maxY = rotatedPoints[0].y;

		for (size_t i = 1; i < rotatedPoints.size(); ++i) {
			minX = std::min(minX, rotatedPoints[i].x);
			maxX = std::max(maxX, rotatedPoints[i].x);
			minY = std::min(minY, rotatedPoints[i].y);
			maxY = std::max(maxY, rotatedPoints[i].y);
		}

		AABB aabb;
		aabb.min = vec2f(offset + vec2f(minX, minY));        // 左下
		aabb.max = vec2f(offset + vec2f(maxX, maxY));        // 右上
		return aabb;
	}
	void draw(vec2f offset = {}) const override {
		Renderer::drawPoly(getPointsRotated(offset), boundary_thickness, boundary_color, fill_color);
	}
};



