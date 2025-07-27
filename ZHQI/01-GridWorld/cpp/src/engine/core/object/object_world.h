#pragma once 
#include "object_screen.h"

class ObjectWorld: public ObjectScreen {
protected:
    glm::vec2 world_position_{}; // 世界坐标
   
public:
    virtual void init() override{type_= ObjectType::World;};
    virtual void update(float dt) override;
    virtual void takeDamage(float) {};

    // setter and getter
    void setWorldPos(const glm::vec2& position);
    virtual glm::vec2 getWorldPos() const override {return world_position_;};

    // 同步设置世界坐标
    virtual void setScreenPos(const glm::vec2& screen_position) override;
};