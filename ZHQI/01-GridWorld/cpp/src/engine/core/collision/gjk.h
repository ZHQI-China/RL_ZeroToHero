#pragma once
#include <engine/util/math.h>
#include "shape.h"

class Simplex {
public:
	std::vector<vec2f> points;

	void add(const vec2f& point) {
		points.push_back(point);
	}

	// 返回true表示包含原点，碰撞；返回false表示不包含，并更新direction
	bool containsOrigin(vec2f& direction) {
		if (points.size() == 1) { // 单纯形是一个点
			// vec2f a = points[0]; (未使用)
			// GJK主循环中已检查过第一个点p是否为原点
			// 这里，如果只有一个点，方向就是从这个点指向原点
			direction = -points[0];
			return false; // 单个点除非是原点本身，否则不包含原点
		}

		if (points.size() == 2) { // 单纯形是一条线段 (A-B)
			vec2f b = points[0]; // 旧点 B
			vec2f a = points[1]; // 新点 A
			vec2f ab = b - a;    // 向量 A->B
			vec2f ao = -a;       // 向量 A->Origin

			// 计算垂直于AB且朝向原点的方向
			// 使用三重积 ((A->B) x (A->O)) x (A->B)
			// 在2D中简化为： perp = (ab.y, -ab.x) 或 (-ab.y, ab.x)
			// 然后确保 perp 指向 ao 的一侧
			vec2f ab_perp = vec2f(ab.y, -ab.x);
			if (Dot(ab_perp, ao) < 0) { // 如果ab_perp和ao点积为负，说明方向反了
				ab_perp = -ab_perp;
			}
			direction = ab_perp;
			return false; // 线段本身不“包含”原点，除非原点在线段上（这由GJK主循环的dot_res<0判断，或添加点时判断）
		}

		// points.size() == 3 (单纯形是一个三角形 A-B-C)
		// A 是最新加入的点 (points[2])
		vec2f c = points[0]; // 最老点 C
		vec2f b = points[1]; // 中间点 B
		vec2f a = points[2]; // 最新点 A

		vec2f ao = -a;    // A -> Origin
		vec2f ab = b - a; // A -> B
		vec2f ac = c - a; // A -> C

		// 处理三角形ABC，A是最新点
		// 检查原点是否在边AB形成的Voronoi区域 (AB外侧，C的另一边)
		vec2f ab_perp = vec2f(ab.y, -ab.x); // AB的一个法向量 (顺时针旋转90度)
		// 将ab_perp调整为指向三角形“外侧”（即远离C的方向）
		if (Dot(ab_perp, c - a) > 0) { // 如果ab_perp和ac同向（指向内）
			ab_perp = -ab_perp;                 // 则反向，使其指向外
		}
		// 如果原点在AB的外侧（由调整后的ab_perp指向）
		if (Dot(ab_perp, ao) > 1e-5f) {
			points.erase(points.begin()); // 移除点C (points[0])
			direction = ab_perp;          // Simplex退化为线段AB，方向是这个外法线
			return false;
		}

		// 检查原点是否在边AC形成的Voronoi区域 (AC外侧，B的另一边)
		vec2f ac_perp = vec2f(ac.y, -ac.x); // AC的一个法向量
		// 将ac_perp调整为指向三角形“外侧”（即远离B的方向）
		if (Dot(ac_perp, b - a) > 0) { // 如果ac_perp和ab同向（指向内）
			ac_perp = -ac_perp;                 // 则反向，使其指向外
		}
		// 如果原点在AC的外侧（由调整后的ac_perp指向）
		if (Dot(ac_perp, ao) > 1e-5f) {
			points.erase(points.begin() + 1); // 移除点B (points[1])
			direction = ac_perp;            // Simplex退化为线段AC，方向是这个外法线
			return false;
		}

		// 如果原点不在AB的外侧区域，也不在AC的外侧区域
		// 则原点在三角形ABC内部
		return true; // 碰撞！
	}
};


static vec2f findFirstDirection() {
	return vec2f(1.0f, 0.0f);
}

static vec2f support(const vec2f& direction, const Shape& shapeA, const Shape& shapeB, vec2f offsetA = {}, vec2f offsetB = {}) {
	vec2f a = shapeA.support(direction, offsetA);
	vec2f b = shapeB.support(-direction, offsetB);
	return a - b;
}

static bool GJK(const Shape& shapeA, const Shape& shapeB, vec2f offsetA = {}, vec2f offsetB = {}) {
	vec2f direction = findFirstDirection();
	Simplex simplex;

	vec2f p = support(direction, shapeA, shapeB, offsetA, offsetB);
	simplex.add(p);

	// 严格来说，GJK第一个点不直接判断是否为原点，而是立即反向
	direction = -p;
	if (std::abs(p.x) < 1e-5f && std::abs(p.y) < 1e-5f) { // 如果第一个闵可夫斯基差值点就是原点
		// 这意味着两个形状的支撑点在原点方向上是相同的，或者它们在接触且接触点在原点。
		// 理论上，如果p是原点，那么a-b=0 => a=b。
		// GJK的通常逻辑是如果p是原点，则dot(p, direction)会是0。
		// 更好的初始检查是在循环开始前，如果p是原点，则相交。
		return true; // 已经接触或重叠在原点
	}

	int iterations = 0; // 防止极端情况下的无限循环
	const int MAX_ITERATIONS = 100; // 或者根据需要调整

	while (iterations++ < MAX_ITERATIONS) { // 添加迭代次数限制
		p = support(direction, shapeA, shapeB);
		float dot_res = Dot(p, direction);

		// std::cout << "p: (" << p.x << ", " << p.y << "), dir: (" << direction.x << ", " << direction.y << "), dot_res: " << dot_res << std::endl;

		if (dot_res < 0.0f) { // 注意这里通常是 < 0 (或 < epsilon)，不是 < -epsilon
			// 如果p没有通过原点，则不相交
			return false;
		}
		// std::cout << dot_res << std::endl; // 移除或注释掉这个调试输出
		simplex.add(p);
		if (simplex.containsOrigin(direction)) {
			return true;
		}
		// 如果 direction 变为零向量，也可能出问题
		if (Dot(direction, direction) < 1e-10f) { // 方向向量过小
			// 这种情况可能意味着原点非常接近单纯形的某个特征，或者数值不稳定
			// 可以尝试基于当前单纯形重新计算一个方向，或者判定为相交/不相交（取决于策略）
			// 例如，如果单纯形是三角形且包含原点附近，可认为相交
			// 如果单纯形是线段且原点在其上，也可认为相交
			// 为了安全，或者如果最后一点p接近原点，可以返回true
			if (simplex.points.size() > 1) return true; // 复杂情况，保守返回true
			return true; // 简单情况，保守返回false
		}
	}
	// std::cerr << "GJK exceeded max iterations" << std::endl;
	return false; // 超过最大迭代次数，认为不相交 (或根据情况处理)
}
