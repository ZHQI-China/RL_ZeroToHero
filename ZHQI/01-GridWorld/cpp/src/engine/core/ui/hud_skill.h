#pragma once

#include "core/object/object_world.h"

class Sprite;

class  HUDSkill : public ObjectWorld
{
protected:
    Sprite* icon_ = nullptr;
    float percentage_ = 1.0f;
public:
    static HUDSkill* addHUDSkillChild(Object* parent, const std::string& file_path, glm::vec2 pos, float scale = 1.0f, Anchor anchor = Anchor::CENTER);
    virtual void render() override;
    // getters and setters
    Sprite* getIcon() const { return icon_; }
    void setIcon(Sprite* icon) { icon_ = icon; }
    float getPercentage() const { return percentage_; }
    void setPercentage(float percentage);
};

