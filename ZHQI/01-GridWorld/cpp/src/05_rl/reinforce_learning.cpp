#include "reinforce_learning.h"

void clearGridType(std::vector<std::vector<int>>& grid_infos, const glm::ivec2& pos, GridType type) {
	int mask = ~static_cast<int>(type); // 生成清除掩码
	grid_infos[pos.x][pos.y] &= mask; // 应用掩码
}

void addGridType(std::vector<std::vector<int>>& grid_infos, const glm::ivec2& pos, GridType type) {
	grid_infos[pos.x][pos.y] |= static_cast<int>(type);
}

bool checkGridType(const std::vector<std::vector<int>>& grid_infos, const glm::ivec2& pos, GridType type) {
	return grid_infos[pos.x][pos.y] & static_cast<int>(type);
}
