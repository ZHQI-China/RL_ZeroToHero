#include "scene_rl.h"
#include <engine/util/asset_store.h>

void SceneRL::init()
{
	tex_player_ = AssetStore::getTexture("./res/imgs/player.png");
	tex_forbidden_ = AssetStore::getTexture("./res/imgs/forbidden.png");
	tex_target_ = AssetStore::getTexture("./res/imgs/target.png");
	tex_arrow_down_ = AssetStore::getTexture("./res/imgs/arrow_down.png");
	tex_stay_ = AssetStore::getTexture("./res/imgs/stay.png");


	ObjectWorld* obj = new ObjectWorld;
	addChild(obj);

	text_map = TextLabel::addTextLabelChild(obj,"Map", R"(C:\WINDOWS\FONTS\MSYH.TTC)", 16);
	text_policy = TextLabel::addTextLabelChild(obj, "Policy", R"(C:\WINDOWS\FONTS\MSYH.TTC)", 16);
	text_state_values = TextLabel::addTextLabelChild(obj, "State Values", R"(C:\WINDOWS\FONTS\MSYH.TTC)", 16);

	vText_state_values.resize(grid_size_.x, std::vector<TextLabel*>(grid_size_.y, nullptr));
	for (int i = 0;i < grid_size_.x;i++) {
		for (int j = 0;j < grid_size_.y;j++) {
			vText_state_values[i][j] = TextLabel::addTextLabelChild(obj, "0", R"(C:\WINDOWS\FONTS\MSYH.TTC)", 16);
		}
	}
}

void SceneRL::handleEvents(SDL_Event& event)
{
	if (event.type == SDL_EVENT_KEY_DOWN) {
		if (event.key.key == SDLK_SPACE) {
			rl_.value_iteration_algorithm(grid_infos_);
			//rl_.policy_iteration_algorithm(grid_infos_);
			//rl_.truncated_iteration_algorithm(grid_infos_);

			//rl_.mc_base_algorithm(grid_infos_);
			//rl_.mc_exploring_start_algorithm(grid_infos_);
			//rl_.mc_epsilon_greedy_algorithm(grid_infos_);

			//rl_.td_base(grid_infos_, player_pos_);//不收敛
			//rl_.td_sarsa(grid_infos_, player_pos_);
			//rl_.td_sarsa_expected(grid_infos_, player_pos_);
			//rl_.td_sarsa_n_step(grid_infos_, player_pos_,100);
			//rl_.q_learning_on_policy(grid_infos_, player_pos_);
			//rl_.q_learning_off_policy(grid_infos_);

			//rl_.vf_table_sarsa(grid_infos_, player_pos_);
			//rl_.vf_sarsa(grid_infos_, player_pos_,rl_.feature_extractor_);
			//rl_.vf_q_learning(grid_infos_, player_pos_, rl_.table_feature_extractor_);


			updateVText_state_values();
			static int count = 1;
			SDL_Log("更新了策略，迭代次数: %d", count++);
		}

		// WASD 控制
		for (int i = 0;i < Keycodes.size();i++) {
			if (event.key.key == Keycodes[i]) {
				takeAction(static_cast<ActionType>(i));
			}
		}

		// 根据策略自动导航
		if (event.key.key == SDLK_RETURN) {//回车
			if (!is_auto_pilot) SDL_Log("开始自动导航");
			else SDL_Log("结束自动导航");
			is_auto_pilot = !is_auto_pilot;
		}
	}
}

void SceneRL::update(float dt)
{
	if (is_auto_pilot) {
		walk_timer += dt;
		if (walk_timer >= walk_time) {
			walk_timer = 0.f;
			auto old_pos = player_pos_;
			int action = rl_.policy_[player_pos_.x][player_pos_.y];
			takeAction(static_cast<ActionType>(action));
			if (old_pos == player_pos_ && checkGridType(grid_infos_, player_pos_, GridType::Target)) {
				SDL_Log("已到达目标，导航结束");
				is_auto_pilot = false;
			}
		}
	}
}

void SceneRL::render()
{
	//Scene::render();
	drawBackground();
	// 遍历二维数组：grid_infos_,根据值放置不同的图像
	for (int row = 0; row < grid_infos_.size(); row++) {
		for (int col = 0; col < grid_infos_[row].size(); col++) {
			int type = grid_infos_[row][col]; // 获取当前格子的类型值

			float angle = 0;
			SDL_FColor tint = { 1.f,1.f,1.f };
			std::vector<std::pair<SDL_Texture*, float>> textures;
			if (show_map_) {
				// 根据类型选择对应的纹理
				if (checkGridType(grid_infos_, { row, col }, GridType::Target)) {
					textures.push_back({ tex_target_ ,angle });
				}
				if (checkGridType(grid_infos_, { row, col }, GridType::Forbidden)) {
					textures.push_back({ tex_forbidden_,angle });
				}
				if (checkGridType(grid_infos_, { row, col }, GridType::Player)) {
					textures.push_back({ tex_player_,angle });
				}
			}
			glm::vec2 pos = glm::vec2(col * gap_width_ + boundary_width_, row * gap_width_ + boundary_width_); // 格子左上角坐标
			glm::vec2 size = glm::vec2(gap_width_, gap_width_);       // 格子大小
			for (auto tex : textures) {// 如果有对应的纹理，绘制到屏幕上
				Renderer::renderTexture(tex.first, pos, size, tex.second, tint);
			}

			if (show_policy_) {

				int dir = rl_.policy_[row][col];
				float xoffset = 1 * grid_size_.y * gap_width_ + 1 * gap_width_;

				if (is_auto_pilot && player_pos_.x == row && player_pos_.y == col) {//高亮策略
					tint = { 1.f,0.f,0.f };
				}
				else {
					tint = { 0.f,1.f,0.f };
				}
				if (dir < 4) {
					angle = dir * 90;
					Renderer::renderTexture(tex_arrow_down_, { pos.x + xoffset,pos.y }, size, angle, tint);
				}
				else {
					Renderer::renderTexture(tex_stay_, { pos.x + xoffset,pos.y }, size, 0, tint);
				}
			}

			if (show_state_values_) {
				float xoffset = 2 * grid_size_.y * gap_width_ + 2 * gap_width_;
				int xxoffset = vText_state_values[row][col]->getTextWidth() / 2;
				//std::cout << row<< "," << col << ": " << vText_state_values[row][col]->getText() << std::endl;
				vText_state_values[row][col]->render(pos.x + xoffset + gap_width_ / 2 - xxoffset, pos.y + gap_width_ / 2);
			}
		}
	}
}
