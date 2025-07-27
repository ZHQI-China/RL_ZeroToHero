#pragma once
#include "core/object/object_world.h"
#include "core/collision/collider.h"

class Stats;
class AffiliateBar;

class  Actor:public ObjectWorld{
protected:
    glm::vec2 velocity_{};                  // 速度
    float max_speed_{250.f};                // 最大速度
    Stats* stats_{};                        // 角色属性
    AffiliateBar* health_bar_ = nullptr;    // 生命条
    Collider* collider_ = nullptr;          // 碰撞器

public:
	virtual void update(float dt);
    void move(float dt);
    virtual void takeDamage(float damage) override;

    // getter and setter
    void setVelocity(const glm::vec2& velocity){ velocity_ = velocity;}
    glm::vec2 getVelocity() const {return velocity_;}
    void setMaxSpeed(float max_speed){max_speed_ = max_speed;}
    float getMaxSpeed() const {return max_speed_;}
    Stats* getStats() const { return stats_; }
    void setStats(Stats* stats) { stats_ = stats; }
    bool getIsAlive() const;

private:
	void updateHealthBar();
};