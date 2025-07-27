#pragma once
#include "core/object/object.h"
class Stats : public Object
{
protected:
    class Actor* parent_ = nullptr;
    float health_ = 100.0f;
    float max_health_ = 100.0f;
    float mana_ = 100.0f;
    float max_mana_ = 100.0f;
    float damage_ = 40.0f;
    float mana_regen_ = 10.0f;

    bool is_invincible_ = false;
    float invincible_time_ = 1.5f;      // 受伤后，无敌时间有多长
    float invincible_timer_ = 0.0f;     // 无敌计时器
    bool is_alive_ = true;

public:
    static Stats* addStatsChild(Actor* parent, float max_health = 100.0f, float max_mana = 100.0f, float damage = 40.0f, float mana_regen = 10.0f);
    virtual void update(float dt) override;

    bool canUseMana(float mana_cost);
    void useMana(float mana_cost);
    void regenMana(float dt);
    void takeDamage(float damage);

    // getters and setters
    float getHealth() const { return health_; }
    float getMaxHealth() const { return max_health_; }
    float getMana() const { return mana_; }
    float getMaxMana() const { return max_mana_; }
    float getDamage() const { return damage_; }
    float getManaRegen() const { return mana_regen_; }
    bool getIsAlive() const { return is_alive_; }
    void setHealth(float health) { health_ = health; }
    void setMaxHealth(float max_health) { max_health_ = max_health; }
    void setMana(float mana) { mana_ = mana; }
    void setMaxMana(float max_mana) { max_mana_ = max_mana; }
    void setDamage(float damage) { damage_ = damage; }
    void setManaRegen(float mana_regen) { mana_regen_ = mana_regen; }
    void setAlive(bool alive) { is_alive_ = alive; }
    void setParent(Actor* parent) { parent_ = parent; }
    Actor* getParent() const { return parent_; }
    void setInvincible(bool invincible) { is_invincible_ = invincible; }
    bool getInvincible() const { return is_invincible_; }
};

