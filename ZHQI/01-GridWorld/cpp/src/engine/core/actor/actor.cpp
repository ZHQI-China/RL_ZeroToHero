#include "actor.h"
#include "core/scene/scene.h"
#include "core/skill/stats.h"
#include "core/affiliate/affiliate_bar.h"

void Actor::move(float dt)
{
    setWorldPos(getWorldPos() + velocity_ * dt);//更新世界坐标和屏幕坐标
    world_position_ = glm::clamp(world_position_, {}, Engine::getInstance().getCurrentScene()->getWorldSize());
    
}
void Actor::update(float dt)
{
	ObjectWorld::update(dt);
	updateHealthBar();
}
void Actor::takeDamage(float damage)
{
    if (!stats_) return;
    stats_->takeDamage(damage);
    //SDL_Log("actor take damage");
}

bool Actor::getIsAlive() const
{
    if (!stats_) return true;
    return stats_->getIsAlive();
}
void Actor::updateHealthBar()
{
	if (!stats_ || !health_bar_) return;
	health_bar_->setPercentage(stats_->getHealth() / stats_->getMaxHealth());
}
