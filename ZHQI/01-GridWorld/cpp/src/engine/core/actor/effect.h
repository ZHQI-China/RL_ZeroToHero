#pragma once
#include "core/object/object_world.h"
class SpriteAnim;
class  Effect : public ObjectWorld
{
    SpriteAnim* sprite_ = nullptr;
    ObjectWorld* next_object_ = nullptr;        // 播放完毕后添加到场景中

public:
    static Effect* addEffectChild(Object* parent, const std::string& file_path, glm::vec2 pos, float scale = 1.0f, ObjectWorld* next_object = nullptr);
    virtual void update(float dt) override;

    // setters and getters
    void setSpriteAnim(SpriteAnim* sprite) { sprite_ = sprite; }
    SpriteAnim* getSpriteAnim() { return sprite_; }
    void setNextObject(ObjectWorld* next_object) { next_object_ = next_object; }
    ObjectWorld* getNextObject() { return next_object_; }

private:
    void checkFinish();
};