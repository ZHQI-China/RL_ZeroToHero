#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <iostream>
#include <fstream>
#include <numeric>

#include <filesystem>
namespace fs = std::filesystem;
#include <functional>

#include <engine/util/randomer.h>

// #include "net/model.h"//引入神经网络
#define M_PI 3.141592654f
enum class GridType {
	None = 0,
	Player = 1 << 0,   // 0001
	Forbidden = 1 << 1, // 0010
	Target = 1 << 2,    // 0100
};

void clearGridType(std::vector<std::vector<int>>& grid_infos, const glm::ivec2& pos, GridType type);
void addGridType(std::vector<std::vector<int>>& grid_infos, const glm::ivec2& pos, GridType type);
bool checkGridType(const std::vector<std::vector<int>>& grid_infos, const glm::ivec2& pos, GridType type);

inline static std::vector<glm::ivec2> Actions{ {1,0},{0,-1},{-1,0},{0,1},{0,0} };//row,col//下，左，上，右，不动
enum class ActionType { Down, Left, Up, Right, Stay };

class ReinforceLearning
{
	friend class SceneRL;
private:
	
	// 表格
	std::vector<std::vector<int>> policy_;  //0-down;1-left;2-up;3-right;4-stay 
	std::vector<std::vector<float>> state_values_;
	std::vector<std::vector<std::vector<float>>> action_values_;
	std::vector<std::vector<std::vector<float>>> action_probs_; //选择动作的概率：在 mc_epsilon_greedy 算法和之后的中使用

	// 值函数->神经网络拟合值函数
	int rows_=0, cols_=0;
	glm::ivec2 target_pos_{};
	std::vector<float> w_;

	// 特征提取器: 输入状态和动作得到特征向量
	using FeatureExtractor = std::function<std::vector<float>(const glm::ivec2&, int)>;

	// 表格特征提取器：vf_table_sarsa
	FeatureExtractor table_feature_extractor_ = [this](const glm::ivec2& pos, int action) {
		int r = pos.x;
		int c = pos.y;
		std::vector<float> features(rows_*cols_*5,0.f);
		
		// 与{r，c}对应的元素为1
		// features[(r*cols_+c)* action] = 1.f;
		int idx = r * cols_ * 5 + c * 5 + action; // 正确索引
		if (idx < features.size()) {
			features[idx] = 1.f;
		}

		return features;
		};

	FeatureExtractor feature_extractor_ = [this](const glm::ivec2& pos, int action) {
		int r = pos.x, c = pos.y;
		std::vector<float> features = {
			// 基础特征
			r / (float)rows_,               // 归一化行坐标 (0~1)
			c / (float)cols_,               // 归一化列坐标 (0~1)

			// 动作特征（One-Hot 编码）
			(action == 0) ? 1.f : 0.f,     // 动作 0
			(action == 1) ? 1.f : 0.f,     // 动作 1
			(action == 2) ? 1.f : 0.f,     // 动作 2
			(action == 3) ? 1.f : 0.f,     // 动作 3
			(action == 4) ? 1.f : 0.f,     // 动作 4

			// 交互特征：状态与动作的组合
			r * (action / 5.f),            // 行坐标与动作的乘积
			c * (action / 5.f),            // 列坐标与动作的乘积

			// 距离特征：到目标点的欧氏距离
			std::sqrtf((r - target_pos_.x) * (r - target_pos_.x) + (c - target_pos_.y) * (c - target_pos_.y)),

			// 方向特征：相对于目标的方向（例如角度）
			atan2f(target_pos_.y - c, target_pos_.x - r) / (2 * M_PI) + 0.5f, // 归一化到 [0,1]

			// 高阶特征：多项式项
			(r / (float)rows_) * (c / (float)cols_), // 行列乘积
			(r / (float)rows_) * (r / (float)rows_), // 行的平方
			(c / (float)cols_) * (c / (float)cols_)  // 列的平方
		};
		return features;
		};

	FeatureExtractor fourier_basis_extractor_ = [this, max_freq = 5](const glm::ivec2& pos, int action) {
		int r = pos.x, c = pos.y;
		float normalized_r = r / (float)rows_;  // 归一化到 [0,1]
		float normalized_c = c / (float)cols_;

		std::vector<float> features;

		// 傅里叶基函数：cos(π * k * normalized_r), sin(π * k * normalized_r), cos(π * k * normalized_c), sin(π * k * normalized_c)
		for (int k = 0; k <= max_freq; ++k) {
			if (k == 0) {
				features.push_back(1.0f);  // 常数项
			}
			else {
				features.push_back(std::cos(M_PI * k * normalized_r));
				features.push_back(std::sin(M_PI * k * normalized_r));
				features.push_back(std::cos(M_PI * k * normalized_c));
				features.push_back(std::sin(M_PI * k * normalized_c));
			}
		}

		// 可选：加入动作 One-Hot 编码
		for (int a = 0; a < 5; ++a) {
			features.push_back(a == action ? 1.0f : 0.0f);
		}

		return features;
		};

	FeatureExtractor tiling_extractor_ = [this, num_tilings = 4, num_tiles = 8](const glm::ivec2& pos, int action) {
		int r = pos.x, c = pos.y;
		std::vector<float> features;

		// 每个 tiling 的偏移量，避免重复
		for (int tiling = 0; tiling < num_tilings; ++tiling) {
			float offset_r = tiling * (1.0f / num_tilings);  // X 方向偏移
			float offset_c = tiling * (1.0f / num_tilings);  // Y 方向偏移

			// 归一化并计算网格索引
			int tile_r = static_cast<int>(std::floor((r / (float)rows_ + offset_r) * num_tiles));
			int tile_c = static_cast<int>(std::floor((c / (float)cols_ + offset_c) * num_tiles));

			// 二进制特征：只激活对应的网格
			int tile_index = tile_r * num_tiles + tile_c;
			features.push_back(1.0f);  // 二进制激活（1 表示激活）
		}

		// 可选：加入动作 One-Hot 编码
		for (int a = 0; a < 5; ++a) {
			features.push_back(a == action ? 1.0f : 0.0f);
		}

		return features;
		};

	// 值函数：线性组合
	std::function<float(
		const std::vector<float>&, const glm::ivec2&, int, 
		const std::function<std::vector<float>(const glm::ivec2&, int)>&
		)> value_function_ =
		[](const std::vector<float>& W, const glm::ivec2& pos, int action, const FeatureExtractor& feature_extractor_)
		{
			auto features = feature_extractor_(pos, action);
			float q = 0.f;
			for (size_t i = 0; i < W.size() && i < features.size(); ++i)
				q += W[i] * features[i];
			return q;
		};

	// 表格值函数对权重的导数
	std::function<std::vector<float>(const glm::ivec2&, int,
		const std::function<std::vector<float>(const glm::ivec2&, int)>&
		)> d_value_function_ =
		[](const glm::ivec2& pos, int action, const FeatureExtractor& feature_extractor_) {
		return feature_extractor_(pos, action);
		};

	// Model model;

public:
	ReinforceLearning() {
		//auto& game = Game::getInstance();
	}
	~ReinforceLearning() {};
	/// 状态值计算常量
	const float DISCOUNT = 0.9f; // 折扣因子
	const int MAX_ITERATIONS = 1e3; // 最大迭代次数
	const int MAX_EPISODES = 25; // 最大路径长度
	const float THRESHOLD = 1e-4; // 收敛阈值
public:
	void init(const std::string& filename, const glm::ivec2& target_pos) {
		if(fs::exists(filename)) loadPolicy(filename);
		// 初始化动作值
		action_values_ = std::vector<std::vector<std::vector<float>>>(
			policy_.size(), std::vector<std::vector<float>>(
				policy_[0].size(), std::vector<float>(5, 0.0f) // 5个动作
			)
		);
		action_probs_ = std::vector<std::vector<std::vector<float>>>(
			policy_.size(), std::vector<std::vector<float>>(
				policy_[0].size(), std::vector<float>({0.2f,0.2f,0.2f,0.2f,0.2f}) // 默认策略是：Stay
			)
		);

		// 初始化状态值，全为0
		state_values_ = std::vector<std::vector<float>>(policy_.size(), std::vector<float>(policy_[0].size(), 0.0f));

		// 初始化
		rows_ = policy_.size();
		if (rows_ > 0) {
			cols_ = policy_[0].size();
		}
		target_pos_ = target_pos;

		// 初始化权重
		w_.resize(512, 0.f);
		//// 初始化模型
		//model.add_layer(LayerType::Input, 3, 1, 1);//输入状态：normalize(row,col,a)
		//model.add_layer(LayerType::Full, 100, 1, 1);
		//model.add_layer(LayerType::Full, 1, 1, 1);//输出:最优动作值
		//model.show_architecture();

	}
private:
	void loadPolicy(const std::string& filename) {
		std::ifstream file(filename);
		if (!file.is_open()) {
			throw std::runtime_error("无法打开文件: " + filename);
		}

		// 读取地图尺寸
		int rows, cols;
		if (!(file >> rows >> cols)) {
			throw std::runtime_error("地图尺寸读取失败");
		}
		// 初始化二维数组
		policy_.resize(rows, std::vector<int>(cols, 0));

		// 读取每一行数据
		for (int r = 0; r < rows; ++r) {
			for (int c = 0; c < cols; ++c) {
				int value;
				if (!(file >> value)) {
					throw std::runtime_error("读取格子值失败，位置 (" + std::to_string(r) + "," + std::to_string(c) + ")");
				}
				policy_[r][c] = value;
			}
		}
	}

	std::pair<int,int> getNextState(const std::vector<std::vector<int>>& grid_infos, int r, int c, int action) {
		int nr = r + Actions[action].x;
		int nc = c + Actions[action].y;

		if (nr < 0 || nr >= grid_infos.size() || nc < 0 || nc >= grid_infos[r].size()) {// 越界
			nr = r;
			nc = c;
		}
		return { nr,nc };
	}

	float getReward(const std::vector<std::vector<int>>& grid_infos, int r, int c, int* nr, int* nc, int action) {
		/*	情况				reward
			x->None			0
			x->Forbidden	-10
			x->Target		1
			x->越界			-1
			// x 表示任意一种情况
		*/
		float reward = 0.0f;
		int next_r = r + Actions[action].x;
		int next_c = c + Actions[action].y;

		if (next_r < 0 || next_r >= grid_infos.size() || next_c < 0 || next_c >= grid_infos[r].size()) {// 越界
			next_r = r;
			next_c = c;
			reward = -1.f;
		}
		else {// 没有越界
			if (checkGridType(grid_infos, { next_r,next_c }, GridType::Forbidden)) {
				reward = -10;
			}
			else if (checkGridType(grid_infos, { next_r,next_c }, GridType::Target)) {
				reward = 1;
			}
			else if (checkGridType(grid_infos, { next_r,next_c }, GridType::None)) {
				reward = 0;
			}
		}
		if (nr) *nr = next_r;
		if (nc) *nc = next_c;
		return reward;
	}

public:// 算法
	/// model-base algorithm
	void value_iteration_algorithm(const std::vector<std::vector<int>>& grid_infos) {
		int rows = grid_infos.size();
		int cols = grid_infos[0].size();

		//for (int iter = 0; iter < MAX_ITERATIONS; ++iter) {
			bool converged = true;
			std::vector<std::vector<float>> new_values = state_values_;

			for (int r = 0; r < rows; ++r) {
				for (int c = 0; c < cols; ++c) {
					for (int action = 0; action < 5; ++action) {
						int nr, nc;
						float reward = getReward(grid_infos, r, c, &nr, &nc, action);
						// 更新动作值
						action_values_[r][c][action] = reward + DISCOUNT * state_values_[nr][nc];
					}
					// 最优化： 选择最大的动作值对应的动作
					int max_action = std::max_element(action_values_[r][c].begin(), action_values_[r][c].end()) - action_values_[r][c].begin();
					
					// 策略更新
					policy_[r][c] = max_action;
					/*action_probs_[r][c] = { 0.f,0.f,0.f,0.f,0.f };
					action_probs_[r][c][max_action] = 1.f;*/

					// 状态值更新
					new_values[r][c] = /*action_probs_[r][c][max_action]**/ action_values_[r][c][max_action];

					if (std::abs(new_values[r][c] - state_values_[r][c]) > THRESHOLD) {// 只要有一个没有收敛就要继续算
						converged = false;
					}
				}
			}

			state_values_ = new_values;
			//if (converged) break;
		//}
	}
	
	void policy_iteration_algorithm(const std::vector<std::vector<int>>& grid_infos) {
		int rows = grid_infos.size();
		int cols = grid_infos[0].size();

		bool policy_converged = true;
		//do {
			// policy evaluation 策略评估：根据当前策略和模型，使用迭代法计算状态值
			for (int iter = 0; iter < MAX_ITERATIONS; ++iter) {
				bool converged = true;
				std::vector<std::vector<float>> new_values = state_values_;

				for (int r = 0; r < rows; ++r) {
					for (int c = 0; c < cols; ++c) {

						int nr, nc;
						float reward = getReward(grid_infos, r, c, &nr, &nc, policy_[r][c]);

						// 更新状态值
						new_values[r][c] = reward + DISCOUNT * state_values_[nr][nc];

						if (std::abs(new_values[r][c] - state_values_[r][c]) > THRESHOLD) {// 只要有一个没有收敛就要继续算
							converged = false;
						}
					}
				}

				state_values_ = new_values;
				if (converged) break;
			}

			// policy improvement 策略提升：
			for (int r = 0; r < rows; r++) {
				for (int c = 0; c < cols; c++) {

					for (int action = 0; action < 5; ++action) {
						int nr, nc;
						float reward = getReward(grid_infos, r, c, &nr, &nc, action);
						// 更新动作值
						action_values_[r][c][action] = reward + DISCOUNT * state_values_[nr][nc];
					}
					// 最优化： 选择最大的并更新策略
					int max_action = std::max_element(action_values_[r][c].begin(), action_values_[r][c].end()) - action_values_[r][c].begin();
					if (policy_[r][c] != max_action) {
						policy_[r][c] = max_action;
						policy_converged = false;
					}
					/*action_probs_[r][c] = { 0.f,0.f,0.f,0.f,0.f };
					action_probs_[r][c][max_action] = 1.f;*/
				}
			}
		//}while (!policy_converged);
	}

	void truncated_iteration_algorithm(const std::vector<std::vector<int>>& grid_infos, int truncated_step=100) {
		int rows = grid_infos.size();
		int cols = grid_infos[0].size();

		bool policy_converged = true;
		//do {
			// policy evaluation 策略评估：根据当前策略和模型，使用迭代法计算状态值
		for (int iter = 0; iter < truncated_step; ++iter) {
			bool converged = true;
			std::vector<std::vector<float>> new_values = state_values_;

			for (int r = 0; r < rows; ++r) {
				for (int c = 0; c < cols; ++c) {

					int nr, nc;
					float reward = getReward(grid_infos, r, c, &nr, &nc, policy_[r][c]);

					// 更新状态值
					new_values[r][c] = reward + DISCOUNT * state_values_[nr][nc];

					if (std::abs(new_values[r][c] - state_values_[r][c]) > THRESHOLD) {// 只要有一个没有收敛就要继续算
						converged = false;
					}
				}
			}

			state_values_ = new_values;
			if (converged) break;
		}

		// policy improvement 策略提升：
		for (int r = 0; r < rows; r++) {
			for (int c = 0; c < cols; c++) {
				for (int action = 0; action < 5; ++action) {
					int nr, nc;
					float reward = getReward(grid_infos, r, c, &nr, &nc, action);
					// 更新动作值
					action_values_[r][c][action] = reward + DISCOUNT * state_values_[nr][nc];
				}
				// 最优化： 选择最大的并更新策略
				int max_action = std::max_element(action_values_[r][c].begin(), action_values_[r][c].end()) - action_values_[r][c].begin();
				if (policy_[r][c] != max_action) {
					policy_[r][c] = max_action;
					policy_converged = false;
				}
			}
		}
		//}while (!policy_converged);
	}

	/// model-free algorithm
	void mc_base_algorithm(const std::vector<std::vector<int>>& grid_infos, int episode_length = 14) {
		int rows = grid_infos.size();
		int cols = grid_infos[0].size();

		bool policy_converged = true;
		//do {
		//policy_converged = true;
		for (int r = 0; r < rows; ++r) {
			for (int c = 0; c < cols; ++c) {

				/// 在每个状态，对各个动作收集一条路径--exploring start
				for (int action = 0;action < 5;action++) {
					int cur_r, cur_c;
					int next_r, next_c;
					float total_reward = getReward(grid_infos, r, c, &cur_r, &cur_c, action); // 这个action表示探索，不根据策略来
					for (int step = 1;step < episode_length; step++) {
						float reward = getReward(grid_infos, cur_r, cur_c, &next_r, &next_c, policy_[cur_r][cur_c]);
						total_reward += std::pow(DISCOUNT, step) * reward;
						cur_r = next_r; cur_c = next_c;
					}
					action_values_[r][c][action] = total_reward / episode_length;
				}
				// every visit mode: 等所有动作采集完再更新策略
				/// 最优化： (随机)选择最大的并更新策略
				int max_action = std::max_element(action_values_[r][c].begin(), action_values_[r][c].end()) - action_values_[r][c].begin();
				
				/*float max_action_value = action_values_[r][c][max_action];
				std::vector<int> max_actions;
				for (int i = 0;i < action_values_.size();i++) {
					if (action_values_[r][c][i] == max_action_value) max_actions.push_back(i);
				}
				max_action = Randomer::randomInt(0, max_actions.size() - 1);*/

				// 更新状态值：只为了显示
				state_values_[r][c] = action_values_[r][c][max_action];// every-visit mode

				if (policy_[r][c] != max_action) {
					policy_[r][c] = max_action;
					policy_converged = false;
				}
			}
		}
		//}while (!policy_converged);
	}

	void mc_exploring_start_algorithm(const std::vector<std::vector<int>>& grid_infos, int episode_length = 14) {
		int rows = grid_infos.size();
		int cols = grid_infos[0].size();
		/// 随机选择状态--去除 exploring start
		int r = Randomer::randomInt(0, rows - 1);
		int c = Randomer::randomInt(0, cols - 1);
		bool policy_converged = true;
		//do {
		//policy_converged = true;

		// 记录每个动作选取的次数
		static std::vector<std::vector<std::vector<int>>> action_count(rows, std::vector<std::vector<int>>(cols, std::vector<int>(5, 0)));

		/// 对于随机选择的这个状态，对各个动作收集一条路径--exploring start
		for (int action = 0;action < 5;action++) {
			auto [cur_r, cur_c] = getNextState(grid_infos, r, c, action); // 这个action表示探索，不根据策略来
			/// 充分利用数据：记录每条路径，从后往前，依次给对应的 action_values_ 赋值
			std::vector<std::tuple<int, int, int>> episode;
			episode.push_back({r,c,action});

			for (int step = 1;step < episode_length; step++) {
				episode.push_back({ cur_r, cur_c, policy_[cur_r][cur_c] });
				auto[next_r, next_c] = getNextState(grid_infos, cur_r, cur_c, policy_[cur_r][cur_c]);
				cur_r = next_r; cur_c = next_c;
			}
			float g = getReward(grid_infos, r, c, nullptr, nullptr, action);
			int count = 0;
			for (int step = episode_length-1;step >= 0;step--) {
				auto [rr, cc, aa] = episode[step];
				action_count[rr][cc][aa]++;
				float reward = getReward(grid_infos, rr, cc, nullptr, nullptr, aa);
				g = DISCOUNT * g + reward;
				action_values_[rr][cc][aa] = action_values_[rr][cc][aa]+g / action_count[rr][cc][aa];

				// first-visit mode: 来一个用一个
				/// 最优化： (随机)选择最大的并更新策略
				int max_action = std::max_element(action_values_[rr][cc].begin(), action_values_[rr][cc].end()) - action_values_[rr][cc].begin();

				/*float max_action_value = action_values_[r][c][max_action];
				std::vector<int> max_actions;
				for (int i = 0;i < action_values_.size();i++) {
					if (action_values_[r][c][i] == max_action_value) max_actions.push_back(i);
				}
				max_action = Randomer::randomInt(0, max_actions.size() - 1);*/

				// 更新状态值：只为了显示
				state_values_[rr][cc] = action_values_[rr][cc][max_action];

				if (policy_[rr][cc] != max_action) {
					policy_[rr][cc] = max_action;
					policy_converged = false;
				}
			}		
		}
		//}while (!policy_converged);
	}

	void mc_epsilon_greedy_algorithm(const std::vector<std::vector<int>>& grid_infos, int episode_length = 100, float epsilon = 0.001f) {
		int rows = grid_infos.size();
		int cols = grid_infos[0].size();

		bool policy_converged = true;
		//do {
		//policy_converged = true;


		/// 随机选择状态--去除 exploring start
		int r = Randomer::randomInt(0, rows -1);
		int c = Randomer::randomInt(0, cols -1);

		/// 在随机选择的这个状态，对各个动作收集一条路径
		for (int action = 0;action < 5;action++) {
			auto [cur_r, cur_c] = getNextState(grid_infos, r, c, action); // 这个action表示探索，不根据策略来

			/// 充分利用数据：记录每条路径，从后往前，依次给对应的 action_values_ 赋值
			std::vector<std::tuple<int, int, int>> episode;
			episode.push_back({ r,c,action });

			for (int step = 1;step < episode_length; step++) {
				episode.push_back({ cur_r, cur_c, policy_[cur_r][cur_c] });
				auto [next_r, next_c] = getNextState(grid_infos, cur_r, cur_c, policy_[cur_r][cur_c]);
				cur_r = next_r; cur_c = next_c;
			}
			float g = 0.f;
			static int count = 0;
			for (int step = episode_length - 1;step >= 0;step--) {
				auto [rr, cc, aa] = episode[step];
				int nr, nc;
				float reward = getReward(grid_infos, rr, cc, &nr, &nc, aa);
				g = DISCOUNT * g + reward;
				action_values_[rr][cc][aa] = (action_values_[rr][cc][aa] + g)/++count;
				
				// first-visit mode: 来一个用一个
				/// 最优化： (随机)选择最大的并更新策略
				int max_action = std::max_element(action_values_[rr][cc].begin(), action_values_[rr][cc].end()) - action_values_[rr][cc].begin();
				action_probs_[rr][cc] = std::vector<float>(5, 1.f / 5 * epsilon);
				action_probs_[rr][cc][max_action] = 1.f - (5.f - 1) / 5 * epsilon;

				/*float max_action_value = action_values_[r][c][max_action];
				std::vector<int> max_actions;
				for (int i = 0;i < action_values_.size();i++) {
					if (action_values_[r][c][i] == max_action_value) max_actions.push_back(i);
				}
				max_action = Randomer::randomInt(0, max_actions.size() - 1);*/

				// 更新状态值：只为了显示
				state_values_[rr][cc] = 0.f;
				for (int i = 0;i < 5;i++) {
					state_values_[rr][cc] += action_probs_[rr][cc][i] * action_values_[rr][cc][i];
				}


				// 按照概率选择策略
				int selet_action = Randomer::spinWheel(action_probs_[rr][cc]);
				if (policy_[rr][cc] != selet_action) {
					policy_[rr][cc] = selet_action;
					policy_converged = false;
				}
			}
		}

		

		//}while (!policy_converged);
	}

	/// 时序差分
	void td_base(const std::vector<std::vector<int>>& grid_infos, const glm::ivec2& player_pos, float alpha = 0.01f, float epsilon = 0.f) {
		

		bool policy_converged = true;
		//do {
		//policy_converged = true;

		/// 从玩家位置出发
		int r = player_pos.x;
		int c = player_pos.y;
		// 探索到目标停止
		while (!checkGridType(grid_infos, { r,c }, GridType::Target)) {
			// 随机选择一个动作
			int action = Randomer::randomInt(0, 3);
			int nr, nc;
			float reward = getReward(grid_infos, r, c, &nr, &nc, action);
			state_values_[r][c] -=  alpha * (state_values_[r][c] - (reward + DISCOUNT * state_values_[nr][nc]));
			action_values_[r][c][action] = reward + state_values_[nr][nc];
			// first-visit mode: 来一个用一个
			/// 最优化： (随机)选择最大的并更新策略
			int max_action = std::max_element(action_values_[r][c].begin(), action_values_[r][c].end()) - action_values_[r][c].begin();
			action_probs_[r][c] = std::vector<float>(5, 1.f / 5 * epsilon);
			action_probs_[r][c][max_action] = 1.f - (5.f - 1) / 5 * epsilon;
			// 按照概率选择策略
			int selet_action = Randomer::spinWheel(action_probs_[r][c]);
			if (policy_[r][c] != selet_action) {
				policy_[r][c] = selet_action;
				policy_converged = false;
			}
			r = nr;
			c = nc;
		}
	}
	void td_sarsa(const std::vector<std::vector<int>>& grid_infos, const glm::ivec2& player_pos, float alpha = 0.1f, float epsilon = 0.001f) {

		bool policy_converged = true;
		//do {
		//policy_converged = true;

		/// 从玩家位置出发
		int r = player_pos.x;
		int c = player_pos.y;
		// 探索到目标停止
		while (!checkGridType(grid_infos, { r,c }, GridType::Target)) {
			// 随机选择一个动作
			int action = Randomer::randomInt(0, 3);
			int nr, nc;
			float reward = getReward(grid_infos, r, c, &nr, &nc, action);
			//int next_action = std::max_element(action_probs_[nr][nc].begin(), action_probs_[nr][nc].end()) - action_probs_[nr][nc].begin();
			
			action_values_[r][c][action] -= alpha * (action_values_[r][c][action] - (reward + DISCOUNT * action_values_[nr][nc][policy_[nr][nc]]));

			// first-visit mode: 来一个用一个
			/// 最优化： 选择最大的并更新策略
			int max_action = std::max_element(action_values_[r][c].begin(), action_values_[r][c].end()) - action_values_[r][c].begin();
			action_probs_[r][c] = std::vector<float>(5, 1.f / 5 * epsilon);
			action_probs_[r][c][max_action] = 1.f - (5.f - 1) / 5 * epsilon;
			// 按照概率选择策略
			int selet_action = Randomer::spinWheel(action_probs_[r][c]);
			if (policy_[r][c] != selet_action) {
				policy_[r][c] = selet_action;
				state_values_[r][c] = action_values_[r][c][selet_action];
				policy_converged = false;
			}
			r = nr;
			c = nc;
		}
	}

	void td_sarsa_expected(const std::vector<std::vector<int>>& grid_infos, const glm::ivec2& player_pos, float alpha = 0.1f, float epsilon = 0.001f) {

		bool policy_converged = true;
		//do {
		//policy_converged = true;

		/// 从玩家位置出发
		int r = player_pos.x;
		int c = player_pos.y;
		// 探索到目标停止
		while (!checkGridType(grid_infos, { r,c }, GridType::Target)) {
			// 随机选择一个动作
			int action = Randomer::randomInt(0, 3);
			int nr, nc;
			float reward = getReward(grid_infos, r, c, &nr, &nc, action);

			// 求期望
			float next_action_value = 0.f;
			for (int i = 0;i < 5;i++) {
				next_action_value += action_probs_[nr][nc][i] * action_values_[nr][nc][i];
			}
			next_action_value /= 5;

			action_values_[r][c][action] -= alpha * (action_values_[r][c][action] - (reward + DISCOUNT * next_action_value));

			// first-visit mode: 来一个用一个
			/// 最优化： 选择最大的并更新策略
			int max_action = std::max_element(action_values_[r][c].begin(), action_values_[r][c].end()) - action_values_[r][c].begin();
			action_probs_[r][c] = std::vector<float>(5, 1.f / 5 * epsilon);
			action_probs_[r][c][max_action] = 1.f - (5.f - 1) / 5 * epsilon;
			// 按照概率选择策略
			int selet_action = Randomer::spinWheel(action_probs_[r][c]);
			if (policy_[r][c] != selet_action) {
				policy_[r][c] = selet_action;
				state_values_[r][c] = action_values_[r][c][selet_action];
				policy_converged = false;
			}
			r = nr;
			c = nc;
		}
	}

	void td_sarsa_n_step(const std::vector<std::vector<int>>& grid_infos, const glm::ivec2& player_pos, int n, float alpha = 0.1f, float epsilon = 0.001f) {

		bool policy_converged = true;
		//do {
		//policy_converged = true;

		/// 从玩家位置出发
		int r = player_pos.x;
		int c = player_pos.y;
		// 探索到目标停止
		while (!checkGridType(grid_infos, { r,c }, GridType::Target)) {
			// 随机选择一个动作
			int action = Randomer::randomInt(0, 3);
			int nr, nc;
			float reward = getReward(grid_infos, r, c, &nr, &nc, action);

			// 求n步，当n->infinty,alpha=1时  ---> mc
			int cur_r, cur_c;
			int next_r, next_c;
			float taget_action_value = getReward(grid_infos, r, c, &cur_r, &cur_c, action); // 这个action表示探索，不根据策略来
			for (int step = 1;step < n; step++) {
				float reward = getReward(grid_infos, cur_r, cur_c, &next_r, &next_c, policy_[cur_r][cur_c]);
				taget_action_value += std::pow(DISCOUNT, step) * reward;
				cur_r = next_r; cur_c = next_c;
			}
			taget_action_value += std::pow(DISCOUNT, n)*action_values_[next_r][next_c][policy_[next_r][next_c]];


			action_values_[r][c][action] -= alpha * (action_values_[r][c][action] - taget_action_value);

			// first-visit mode: 来一个用一个
			/// 最优化： 选择最大的并更新策略
			int max_action = std::max_element(action_values_[r][c].begin(), action_values_[r][c].end()) - action_values_[r][c].begin();
			action_probs_[r][c] = std::vector<float>(5, 1.f / 5 * epsilon);
			action_probs_[r][c][max_action] = 1.f - (5.f - 1) / 5 * epsilon;
			// 按照概率选择策略
			int selet_action = Randomer::spinWheel(action_probs_[r][c]);
			if (policy_[r][c] != selet_action) {
				policy_[r][c] = selet_action;
				state_values_[r][c] = action_values_[r][c][selet_action];
				policy_converged = false;
			}
			r = nr;
			c = nc;
		}
	}
	void q_learning_on_policy(const std::vector<std::vector<int>>& grid_infos, const glm::ivec2& player_pos, float alpha = 0.1f, float epsilon = 0.001f) {

		bool policy_converged = true;
		//do {
		//policy_converged = true;

		/// 从玩家位置出发
		int r = player_pos.x;
		int c = player_pos.y;
		// 探索到目标停止
		while (!checkGridType(grid_infos, { r,c }, GridType::Target)) {
			// 随机选择一个动作
			int action = Randomer::randomInt(0, 3);
			int nr, nc;
			float reward = getReward(grid_infos, r, c, &nr, &nc, action);

			// 最优化动作
			int next_action = std::max_element(action_values_[nr][nc].begin(), action_values_[nr][nc].end()) - action_values_[nr][nc].begin();

			action_values_[r][c][action] -= alpha * (action_values_[r][c][action] - (reward + DISCOUNT * action_values_[nr][nc][next_action]));

			// first-visit mode: 来一个用一个
			/// 最优化： 选择最大的并更新策略
			int max_action = std::max_element(action_values_[r][c].begin(), action_values_[r][c].end()) - action_values_[r][c].begin();
			action_probs_[r][c] = std::vector<float>(5, 1.f / 5 * epsilon);
			action_probs_[r][c][max_action] = 1.f - (5.f - 1) / 5 * epsilon;
			// 按照概率选择策略
			int selet_action = Randomer::spinWheel(action_probs_[r][c]);
			if (policy_[r][c] != selet_action) {
				policy_[r][c] = selet_action;
				state_values_[r][c] = action_values_[r][c][selet_action];
				policy_converged = false;
			}
			r = nr;
			c = nc;
		}
	}
	void q_learning_off_policy(const std::vector<std::vector<int>>& grid_infos, float alpha = 0.1f) {
		int rows = grid_infos.size();
		int cols = grid_infos[0].size();
		bool policy_converged = true;
		//do {
		//policy_converged = true;

		/// 不从玩家位置出发，而是随机选择-----off policy--->异策略
		int r = Randomer::randomInt(0, rows - 1);
		int c = Randomer::randomInt(0, cols - 1);
		// 探索到目标停止
		while (!checkGridType(grid_infos, { r,c }, GridType::Target)) {
			// 随机选择一个动作
			int action = Randomer::randomInt(0, 3);
			int nr, nc;
			float reward = getReward(grid_infos, r, c, &nr, &nc, action);

			// 最优化动作-
			int next_action = std::max_element(action_values_[nr][nc].begin(), action_values_[nr][nc].end()) - action_values_[nr][nc].begin();

			action_values_[r][c][action] -= alpha * (action_values_[r][c][action] - (reward + DISCOUNT * action_values_[nr][nc][next_action]));

			// first-visit mode: 来一个用一个
			/// 最优化： 选择最大的并更新策略
			int max_action = std::max_element(action_values_[r][c].begin(), action_values_[r][c].end()) - action_values_[r][c].begin();
			if (policy_[r][c] != max_action) {
				policy_[r][c] = max_action;
				state_values_[r][c] = action_values_[r][c][max_action];
				policy_converged = false;
			}
			r = nr;
			c = nc;
		}
	}

	/// 值函数 value function
	// 用值函数模拟表格法
	void vf_table_sarsa(const std::vector<std::vector<int>>& grid_infos, const glm::ivec2& player_pos, float alpha = 0.001f, float epsilon = 0.001f) {
		
		bool policy_converged = true;
		//do {
		//policy_converged = true;

		/// 从玩家位置出发
		int r = player_pos.x;
		int c = player_pos.y;
		// 探索到目标停止
		while (!checkGridType(grid_infos, { r,c }, GridType::Target)) {
			// 随机选择一个动作
			int action = Randomer::randomInt(0, 3);
			int nr, nc;
			float reward = getReward(grid_infos, r, c, &nr, &nc, action);
			//int next_action = std::max_element(action_probs_[nr][nc].begin(), action_probs_[nr][nc].end()) - action_probs_[nr][nc].begin();

			float qt = value_function_(w_, { r,c }, action, table_feature_extractor_);
			//// 最优化动作-
			//int next_action = std::max_element(action_values_[nr][nc].begin(), action_values_[nr][nc].end()) - action_values_[nr][nc].begin();
			int next_action = policy_[nr][nc];
			float qt1 = value_function_(w_, { nr,nc }, next_action, table_feature_extractor_); // 当前策略选择下一动作

			std::vector<float> dw = d_value_function_({ r,c }, action, table_feature_extractor_);
			float td_error = reward + DISCOUNT * qt1 - qt;

			//action_values_[r][c][action] -= alpha * (action_values_[r][c][action] - (reward + DISCOUNT * action_values_[nr][nc][policy_[nr][nc]]));
			// 更新权重 w_
			for (int i = 0; i < dw.size(); ++i) {
				w_[i] += alpha * td_error * dw[i];
				//float dw_i = dw[i];
				//printf("dw_%d: %f\n", i, dw_i);
				//w_[i] -= alpha * (value_function_(w_, { r,c }, action) - (reward + DISCOUNT * value_function_(w_, { nr,nc }, policy_[nr][nc]))) * dw_i;
			}
			//int idx = r * cols_ * 5 + c * 5 + action; // 正确索引
			//float dw_i = dw[idx];
			//w_[idx] -= alpha * (value_function_(w_, { r,c }, action) - (reward + DISCOUNT * value_function_(w_, { nr,nc }, policy_[nr][nc]))) * dw_i;

			action_values_[r][c][action] = value_function_(w_, { r,c }, action, table_feature_extractor_);


			// first-visit mode: 来一个用一个
			/// 最优化： 选择最大的并更新策略
			int max_action = std::max_element(action_values_[r][c].begin(), action_values_[r][c].end()) - action_values_[r][c].begin();
			action_probs_[r][c] = std::vector<float>(5, 1.f / 5 * epsilon);
			action_probs_[r][c][max_action] = 1.f - (5.f - 1) / 5 * epsilon;
			// 按照概率选择策略
			int selet_action = Randomer::spinWheel(action_probs_[r][c]);
			if (policy_[r][c] != selet_action) {
				policy_[r][c] = selet_action;
				state_values_[r][c] = action_values_[r][c][selet_action];
				policy_converged = false;
			}
			r = nr;
			c = nc;
		}
		
	}
	void vf_sarsa(const std::vector<std::vector<int>>& grid_infos, const glm::ivec2& player_pos, const FeatureExtractor& fe, float alpha = 0.001f, float epsilon = 0.001f) {

		bool policy_converged = true;
		//do {
		//policy_converged = true;

		/// 从玩家位置出发
		int r = player_pos.x;
		int c = player_pos.y;
		// 探索到目标停止
		while (!checkGridType(grid_infos, { r,c }, GridType::Target)) {
			// 随机选择一个动作
			int action = Randomer::randomInt(0, 3);
			int nr, nc;
			float reward = getReward(grid_infos, r, c, &nr, &nc, action);
			//int next_action = std::max_element(action_probs_[nr][nc].begin(), action_probs_[nr][nc].end()) - action_probs_[nr][nc].begin();

			float qt = value_function_(w_, { r,c }, action, fe);
			//// 最优化动作-
			//int next_action = std::max_element(action_values_[nr][nc].begin(), action_values_[nr][nc].end()) - action_values_[nr][nc].begin();
			int next_action = policy_[nr][nc];
			float qt1 = value_function_(w_, { nr,nc }, next_action, fe); // 当前策略选择下一动作

			std::vector<float> dw = d_value_function_({ r,c }, action, fe);
			float td_error = reward + DISCOUNT * qt1 - qt;

			//action_values_[r][c][action] -= alpha * (action_values_[r][c][action] - (reward + DISCOUNT * action_values_[nr][nc][policy_[nr][nc]]));
			// 更新权重 w_
			for (int i = 0; i < dw.size(); ++i) {
				w_[i] += alpha * td_error * dw[i];
				//float dw_i = dw[i];
				//printf("dw_%d: %f\n", i, dw_i);
				//w_[i] -= alpha * (value_function_(w_, { r,c }, action) - (reward + DISCOUNT * value_function_(w_, { nr,nc }, policy_[nr][nc]))) * dw_i;
			}
			//int idx = r * cols_ * 5 + c * 5 + action; // 正确索引
			//float dw_i = dw[idx];
			//w_[idx] -= alpha * (value_function_(w_, { r,c }, action) - (reward + DISCOUNT * value_function_(w_, { nr,nc }, policy_[nr][nc]))) * dw_i;

			action_values_[r][c][action] = value_function_(w_, { r,c }, action, fe);


			// first-visit mode: 来一个用一个
			/// 最优化： 选择最大的并更新策略
			int max_action = std::max_element(action_values_[r][c].begin(), action_values_[r][c].end()) - action_values_[r][c].begin();
			action_probs_[r][c] = std::vector<float>(5, 1.f / 5 * epsilon);
			action_probs_[r][c][max_action] = 1.f - (5.f - 1) / 5 * epsilon;
			// 按照概率选择策略
			int selet_action = Randomer::spinWheel(action_probs_[r][c]);
			if (policy_[r][c] != selet_action) {
				policy_[r][c] = selet_action;
				state_values_[r][c] = action_values_[r][c][selet_action];
				policy_converged = false;
			}
			r = nr;
			c = nc;
		}

	}
	void vf_q_learning(const std::vector<std::vector<int>>& grid_infos, const glm::ivec2& player_pos, const FeatureExtractor& fe, float alpha = 0.001f, float epsilon = 0.001f) {

		bool policy_converged = true;
		//do {
		//policy_converged = true;

		/// 从玩家位置出发
		int r = player_pos.x;
		int c = player_pos.y;
		// 探索到目标停止
		while (!checkGridType(grid_infos, { r,c }, GridType::Target)) {
			// 随机选择一个动作
			int action = Randomer::randomInt(0, 3);
			int nr, nc;
			float reward = getReward(grid_infos, r, c, &nr, &nc, action);

			//int next_action = policy_[nr][nc];
			//// 最优化动作
			int next_action = std::max_element(action_values_[nr][nc].begin(), action_values_[nr][nc].end()) - action_values_[nr][nc].begin();
			//int next_action = std::max_element(action_probs_[nr][nc].begin(), action_probs_[nr][nc].end()) - action_probs_[nr][nc].begin();
			
			float qt = value_function_(w_, { r,c }, action, fe);
			float qt1 = value_function_(w_, { nr,nc }, next_action, fe); // 当前策略选择下一动作

			std::vector<float> dw = d_value_function_({ r,c }, action, fe);
			float td_error = reward + DISCOUNT * qt1 - qt;

			// 更新权重 w_
			for (int i = 0; i < dw.size(); ++i) {
				w_[i] += alpha * td_error * dw[i];
			}
			
			action_values_[r][c][action] = value_function_(w_, { r,c }, action, fe);


			// first-visit mode: 来一个用一个
			/// 最优化： 选择最大的并更新策略
			int max_action = std::max_element(action_values_[r][c].begin(), action_values_[r][c].end()) - action_values_[r][c].begin();
			action_probs_[r][c] = std::vector<float>(5, 1.f / 5 * epsilon);
			action_probs_[r][c][max_action] = 1.f - (5.f - 1) / 5 * epsilon;
			// 按照概率选择策略
			int selet_action = Randomer::spinWheel(action_probs_[r][c]);
			if (policy_[r][c] != selet_action) {
				policy_[r][c] = selet_action;
				state_values_[r][c] = action_values_[r][c][selet_action];
				policy_converged = false;
			}
			r = nr;
			c = nc;
		}

	}
	
	void deep_q_learning(const std::vector<std::vector<int>>& grid_infos, const glm::ivec2& player_pos, float alpha = 0.001f, float epsilon = 0.001f) {

		bool policy_converged = true;
		//do {
		//policy_converged = true;

		/// 从玩家位置出发
		int r = player_pos.x;
		int c = player_pos.y;
		// 探索到目标停止
		while (!checkGridType(grid_infos, { r,c }, GridType::Target)) {
			// 随机选择一个动作
			int action = Randomer::randomInt(0, 3);
			int nr, nc;
			float reward = getReward(grid_infos, r, c, &nr, &nc, action);

			//int next_action = policy_[nr][nc];
			//// 最优化动作
			int next_action = std::max_element(action_values_[nr][nc].begin(), action_values_[nr][nc].end()) - action_values_[nr][nc].begin();
			//int next_action = std::max_element(action_probs_[nr][nc].begin(), action_probs_[nr][nc].end()) - action_probs_[nr][nc].begin();

			/*model.forward({ { (double)r / rows_ }, {(double)c / cols_}, {(double)action / 5} });
			auto [_,probs] = model.predict({ { (double)r / rows_ }, {(double)c / cols_}, {(double)action / 5} });
			model.backward({ {{ reward+DISCOUNT* probs[0]}}});


			action_values_[r][c][action] = (float)probs[0];*/


			// first-visit mode: 来一个用一个
			/// 最优化： 选择最大的并更新策略
			int max_action = std::max_element(action_values_[r][c].begin(), action_values_[r][c].end()) - action_values_[r][c].begin();
			action_probs_[r][c] = std::vector<float>(5, 1.f / 5 * epsilon);
			action_probs_[r][c][max_action] = 1.f - (5.f - 1) / 5 * epsilon;
			// 按照概率选择策略
			int selet_action = Randomer::spinWheel(action_probs_[r][c]);
			if (policy_[r][c] != selet_action) {
				policy_[r][c] = selet_action;
				state_values_[r][c] = action_values_[r][c][selet_action];
				policy_converged = false;
			}
			r = nr;
			c = nc;
		}

	}

	// todo: 引入无监督神经网络
};
using RL = ReinforceLearning;