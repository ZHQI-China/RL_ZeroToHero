#pragma once
#include <engine/core/scene/scene.h>
#include <engine/util/renderer.h>
#include <engine/util/utils.h>
#include <engine/core/affiliate/text_label.h>
#include "reinforce_learning.h"

class SceneRL : public Scene {
public:
	SceneRL(const std::string& name, const std::string& map_file, const std::string& policy_file="") {
		name_ = name;
		// 读取地图文件
		loadMap(map_file);
		printMap();
		if (!fs::exists(policy_file)) {
			rl_.policy_.resize(grid_size_.x, std::vector<int>(grid_size_.y, static_cast<int>(ActionType::Stay)));
		}
		rl_.init(policy_file, target_pos_);
	}
public:
	void init() override;
	void handleEvents(SDL_Event& event) override;
	void update(float dt) override;
	void render() override;
private:
	/// 场景信息
	bool show_map_ = true;
	bool is_auto_pilot = false;
	float walk_timer = 0.f;
	const float walk_time = 0.5f;
	std::vector<std::vector<int>> grid_infos_;  //0-None;1-Player;2-Forbidden;4-Target 
	inline static std::vector<SDL_Keycode> Keycodes{ SDLK_S,SDLK_A,SDLK_W,SDLK_D };
	glm::ivec2 player_pos_{ 0,0 };//row,col
	glm::ivec2 target_pos_{ 0,0 };//row,col
	glm::ivec2 grid_size_{ 10,10 };
	int boundary_width_ = 4;
	int gap_width_ = 50;


	SDL_Texture* tex_player_ = nullptr;
	SDL_Texture* tex_forbidden_ = nullptr;
	SDL_Texture* tex_target_ = nullptr;
	SDL_Texture* tex_arrow_down_ = nullptr;
	SDL_Texture* tex_stay_ = nullptr;

	TextLabel* text_map = nullptr;
	TextLabel* text_policy = nullptr;
	TextLabel* text_state_values = nullptr;

	// 强化学习模块
	bool show_policy_ = true;
	bool show_state_values_ = true;
	ReinforceLearning rl_;

	std::vector<std::vector<TextLabel*>> vText_state_values;

private:

	void drawBackground(SDL_FColor grid_color = { 1.f,1.f,1.f,1.f }, SDL_FColor boundary_color = { 1.f,1.f,1.f,1.f })
	{
		for (int i = 0;i < 3;i++) {
			float xoffset = i * grid_size_.y * gap_width_ + i * gap_width_;
			glm::vec2 start{ boundary_width_ + xoffset,boundary_width_ };
			glm::vec2 end{ boundary_width_ + grid_size_.y * gap_width_ + xoffset,boundary_width_ + grid_size_.x * gap_width_ };
			Renderer::drawGrid(start, end, gap_width_, grid_color);
			Renderer::drawBoundary(start, end, boundary_width_, boundary_color);


			TextLabel* text = nullptr;
			switch (i)
			{
			case 0:
				text = text_map;
				break;
			case 1:
				text = text_policy;
				break;
			case 2:
				text = text_state_values;
				break;
			}
			glm::vec2 text_pos = { end.x - grid_size_.y * gap_width_ / 2 - text->getTextWidth() / 2,end.y };
			text->render(text_pos.x, text_pos.y);
		}
	}


	void loadMap(const std::string& filename) {
		std::ifstream file(filename);
		if (!file.is_open()) {
			throw std::runtime_error("无法打开文件: " + filename);
		}

		// 读取地图尺寸
		int rows, cols;
		if (!(file >> rows >> cols)) {
			throw std::runtime_error("地图尺寸读取失败");
		}
		grid_size_ = { rows,cols };
		// 初始化二维数组
		grid_infos_.resize(rows, std::vector<int>(cols, 0));

		// 读取每一行数据
		for (int r = 0; r < rows; ++r) {
			for (int c = 0; c < cols; ++c) {
				int value;
				if (!(file >> value)) {
					throw std::runtime_error("读取格子值失败，位置 (" + std::to_string(r) + "," + std::to_string(c) + ")");
				}
				if (value == 1) player_pos_ = { r,c };
				if (value == 4) target_pos_ = { r,c };
				grid_infos_[r][c] = value;
			}
		}
	}

	void saveMap(const std::string& filename) {
		std::ofstream file(filename);
		if (!file.is_open()) {
			throw std::runtime_error("无法打开文件: " + filename);
		}

		int rows = grid_infos_.size();
		if (rows == 0) {
			throw std::runtime_error("地图为空");
		}
		int cols = grid_infos_[0].size();

		file << rows << " " << cols << "\n";

		for (int r = 0; r < rows; ++r) {
			for (int c = 0; c < cols; ++c) {
				file << grid_infos_[r][c] << " ";
			}
			file << "\n";
		}
	}

	void printMap() {
		for (int r = 0; r < grid_size_.x; ++r) {
			for (int c = 0; c < grid_size_.y; ++c) {
				std::cout << grid_infos_[r][c] << " ";
			}
			std::cout << "\n";
		}
		std::cout << "\n";
	}

	void takeAction(ActionType type) {
		clearGridType(grid_infos_, player_pos_, GridType::Player);
		player_pos_ += Actions[static_cast<int>(type)];
		player_pos_ = glm::clamp(player_pos_, { 0,0 }, grid_size_ - 1);
		addGridType(grid_infos_, player_pos_, GridType::Player);
	}


	void updateVText_state_values() {
		for (int r = 0;r < grid_size_.x;r++) {
			for (int c = 0;c < grid_size_.y;c++) {
				vText_state_values[r][c]->setText(to_string_with_precision(rl_.state_values_[r][c]));
				//std::cout << vText_state_values[r][c]->getText() << std::endl;
			}
		}
	}

};