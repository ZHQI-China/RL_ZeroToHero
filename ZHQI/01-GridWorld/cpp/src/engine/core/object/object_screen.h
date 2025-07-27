#pragma once

#include "object.h"

class ObjectScreen:public Object{
protected:
    glm::vec2 screen_position_{};// 渲染(屏幕)位置

public:
    virtual void init() override{type_ = ObjectType::Screen;};
    // getter and setter
    glm::vec2 getScreenPos() const{return screen_position_;};
    virtual void setScreenPos(const glm::vec2& screen_position){screen_position_= screen_position;};
    virtual glm::vec2 getWorldPos() const { return {}; };
};