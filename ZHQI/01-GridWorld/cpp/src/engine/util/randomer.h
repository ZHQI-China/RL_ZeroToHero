#pragma once
#include "global/defs.h"
#include <random>
#include <glm/glm.hpp>

class Randomer final{
private://禁止实例化
	Randomer() = delete;
	~Randomer() = delete;
private:
	inline static std::mt19937 gen_ = std::mt19937(std::random_device{}()); //随机数引擎
public:
	static auto getEngine() { return gen_; }
	static float randomFloat(float min, float max) { return std::uniform_real_distribution<float>(min, max)(gen_); }
	static int randomInt(int min, int max) { return std::uniform_int_distribution<int>(min, max)(gen_); }
	static glm::vec2 randomVec2(const glm::vec2& min, const glm::vec2& max) { return glm::vec2(randomFloat(min.x, max.x), randomFloat(min.y, max.y)); }
	static glm::ivec2 randomIVec2(const glm::ivec2& min, const glm::ivec2& max) { return glm::ivec2(randomInt(min.x, max.x), randomInt(min.y, max.y)); }
	// 轮盘赌
	static int spinWheel(const std::vector<float>& probs) {
		// Step 1: 验证输入
		if (probs.empty()) {
			throw std::invalid_argument("Probability vector cannot be empty.");
		}

		// Step 2: 归一化概率
		float total = 0.0f;
		for (float p : probs) {
			if (p < 0) {
				throw std::invalid_argument("All probabilities must be non-negative.");
			}
			total += p;
		}

		if (total == 0.0f) {
			throw std::invalid_argument("Total probability cannot be zero.");
		}

		std::vector<float> normalized_probs;
		for (float p : probs) {
			normalized_probs.push_back(p / total);
		}

		// Step 3: 构建累积概率分布
		std::vector<float> cumulative;
		float sum = 0.0f;
		for (float p : normalized_probs) {
			sum += p;
			cumulative.push_back(sum);
		}

		// Step 4: 生成随机数
		float r = randomFloat(0.f, 1.f);

		// Step 5: 查找匹配的区间
		auto it = std::lower_bound(cumulative.begin(), cumulative.end(), r);
		if (it != cumulative.end()) {
			return std::distance(cumulative.begin(), it);
		}
		else {
			// 防止浮点精度误差导致越界
			return probs.size() - 1;
		}
	}
};