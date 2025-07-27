#include <engine/global/engine.h>
#include <engine/util/logger.h>
#include <engine/util/renderer.h>


#include "scene_rl.h"


int main(int, char**) {
	auto& engine = Engine::getInstance();
	engine.init("引擎沙盒测试-强化学习算法演示", 1200, 720);

	engine.addScene(new SceneRL("强化学习算法演示", "./res/map5x5.txt"/*,"./res/policy3x3.txt"*/));
	engine.setCurrentScene("强化学习算法演示");

	engine.run();
	return 0;
}
