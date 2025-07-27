#pragma once

#include "core/object/object_world.h"

class Sprite;
class Actor;
class  HUDStats : public ObjectWorld
{
protected:
    Actor* target_ = nullptr;
    Sprite* health_bar_ = nullptr;
    Sprite* health_bar_bg_ = nullptr;
    Sprite* health_icon_ = nullptr;
    Sprite* mana_bar_ = nullptr;
    Sprite* mana_bar_bg_ = nullptr;
    Sprite* mana_icon_ = nullptr;
    
    float health_percentage_ = 1.0f;
    float mana_percentage_ = 1.0f;

    // TODO: 更加细致设置大小、位置的变量

public:
    static HUDStats* addHUDStatsChild(Object* parent, Actor* target, glm::vec2 render_position);
    virtual void init() override;
    virtual void update(float dt) override;
    // setters and getters
    void setTarget(Actor* target) { target_ = target; }
    Actor* getTarget() const { return target_; }
    Sprite* getHealthBar() const { return health_bar_; }
    Sprite* getHealthBarBg() const { return health_bar_bg_; }
    Sprite* getHealthIcon() const { return health_icon_; }
    Sprite* getManaBar() const { return mana_bar_; }
    Sprite* getManaBarBg() const { return mana_bar_bg_; }
    Sprite* getManaIcon() const { return mana_icon_; }

    float getHealthPercentage() const { return health_percentage_; }
    float getManaPercentage() const { return mana_percentage_; }
    void setHealthPercentage(float percentage) { health_percentage_ = percentage; }
    void setManaPercentage(float percentage) { mana_percentage_ = percentage; }

private:
    void update_health_bar();
    void update_mana_bar();
};
