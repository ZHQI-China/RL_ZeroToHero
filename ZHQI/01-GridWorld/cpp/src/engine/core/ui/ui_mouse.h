#pragma once
#include "core/object/object_screen.h"
#include "core/affiliate/sprite.h"

class UIMouse : public ObjectWorld
{
protected:
    Sprite* sprite1_ = nullptr;
    Sprite* sprite2_ = nullptr;
    float timer_ = 0;
public:
    static UIMouse* addUIMouseChild(Object* parent, const std::string& file_path1, const std::string& file_path2, float scale = 1.0f, Anchor anchor = Anchor::CENTER);
    virtual void update(float dt) override;

    // setters and getters
    Sprite* getSprite1() const { return sprite1_; }
    Sprite* getSprite2() const { return sprite2_; }
    void setSprite1(Sprite* sprite) { sprite1_ = sprite; }
    void setSprite2(Sprite* sprite) { sprite2_ = sprite; }
};