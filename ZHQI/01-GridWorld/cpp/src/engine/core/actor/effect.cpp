#include "effect.h"
#include "core/scene/scene.h"
#include "core/affiliate/sprite_anim.h"

Effect* Effect::addEffectChild(Object* parent, const std::string& file_path, glm::vec2 pos, float scale, ObjectWorld* next_object)
{
    auto effect = new Effect();
    effect->init();
    effect->sprite_ = SpriteAnim::addSpriteAnimChild(effect, file_path, scale);
    effect->sprite_->setLoop(false);
    effect->setWorldPos(pos);
    effect->setNextObject(next_object);
    if(parent) parent->safeAddChild(effect);
    return effect;
}

void Effect::update(float dt)
{
    ObjectWorld::update(dt);
    checkFinish();
}


void Effect::checkFinish()
{
    if (sprite_->getFinish())
    {
        need_remove_ = true;
        if (next_object_) {
            Engine::getInstance().getCurrentScene()->safeAddChild(next_object_);
        }
    }
}