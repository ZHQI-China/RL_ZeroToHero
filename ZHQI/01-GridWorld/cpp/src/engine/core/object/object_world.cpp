#include "object_world.h"
#include "core/scene/scene.h"
void ObjectWorld::update(float dt)
{
    Object::update(dt);
    screen_position_ = Engine::getInstance().getCurrentScene()->worldToScreen(world_position_);// 同步更新世界坐标和渲染坐标
}
void ObjectWorld::setWorldPos(const glm::vec2 &position)
{
    world_position_ = position;
    screen_position_ = Engine::getInstance().getCurrentScene()->worldToScreen(world_position_);
}

void ObjectWorld::setScreenPos(const glm::vec2 & screen_position)
{
    screen_position_ = screen_position;
    world_position_ = Engine::getInstance().getCurrentScene()->screenToWorld(screen_position_);
}
