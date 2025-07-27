#include "spawner.h"
//#include "enemy.h"
//#include "player.h"
//#include "effect.h"
//#include "../scene/scene.h"
//
//void Spawner::update(float dt)
//{
//    if (!target_ || !target_->getActive()) return;
//    timer_ += dt;
//    if (timer_ >= interval_)
//    {
//        timer_ = 0;
//        for (int i = 0; i < num_; i++)
//        {
//            // spawn enemy
//            auto pos = engine_.randomVec2(engine_.getCurrentScene()->getCameraPosition(), engine_.getCurrentScene()->getCameraPosition() + engine_.getScreenSize());
//            Enemy* enemy = Enemy::addEnemyChild(nullptr, pos, target_);
//            Effect::addEffectChild(engine_.getCurrentScene(), "src/sandbox/assets/effect/184_3.png", pos, 1.0f, enemy);
//            // SDL_Log("engine_.getCurrentScene()->getChildrenWorld().size()=%d", engine_.getCurrentScene()->getChildrenWorld().size());
//        }
//    }
//}

