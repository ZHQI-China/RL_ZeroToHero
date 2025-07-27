#pragma once
#include "core/object/object_world.h"
#include "core/affiliate/sprite_anim.h"

class Spell : public ObjectWorld
{
protected:
    SpriteAnim* sprite_ = nullptr;
    float damage_ = 60.0f;
public:
    static Spell* addSpellChild(Object* parent, const std::string& file_path, glm::vec2 pos, float damage, float scale = 1.0f, Anchor anchor = Anchor::CENTER);
    void update(float dt) override;


    // getters and setters
    float getDamage() const { return damage_; }
    void setDamage(float damage) { damage_ = damage; }
    SpriteAnim* getSpriteAnim() const { return sprite_; }
    void setSpriteAnim(SpriteAnim* sprite) { sprite_ = sprite; }

private:
    void attack();
};