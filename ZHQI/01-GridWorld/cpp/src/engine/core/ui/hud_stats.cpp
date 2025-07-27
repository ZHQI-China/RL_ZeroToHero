#include "hud_stats.h"
#include "core/actor/actor.h"
#include "core/skill/stats.h"
#include "core/affiliate/sprite.h"

HUDStats* HUDStats::addHUDStatsChild(Object* parent, Actor* target, glm::vec2 render_position) {
    HUDStats* hud_stats = new HUDStats();
    hud_stats->init();
   
    hud_stats->setScreenPos(render_position);
    hud_stats->setTarget(target);
    if (parent) parent->addChild(hud_stats);
    return hud_stats;
}

void HUDStats::init()
{
    ObjectWorld::init();
    health_bar_bg_ = Sprite::addSpriteChild(this, "assets/UI/bar_bg.png", 3.0f, Anchor::CENTER_LEFT);
    health_bar_bg_->setOffset(health_bar_bg_->getOffset() + glm::vec2(30, 0));
    health_bar_ = Sprite::addSpriteChild(this, "assets/UI/bar_red.png", 3.0f, Anchor::CENTER_LEFT);
    health_bar_->setOffset(health_bar_->getOffset() + glm::vec2(30, 0));
    health_icon_ = Sprite::addSpriteChild(this, "assets/UI/Red Potion.png", 0.5f, Anchor::CENTER_LEFT);

    mana_bar_bg_ = Sprite::addSpriteChild(this, "assets/UI/bar_bg.png", 3.0f, Anchor::CENTER_LEFT);
    mana_bar_bg_->setOffset(mana_bar_bg_->getOffset() + glm::vec2(300, 0));
    mana_bar_ = Sprite::addSpriteChild(this, "assets/UI/bar_blue.png", 3.0f, Anchor::CENTER_LEFT);
    mana_bar_->setOffset(mana_bar_->getOffset() + glm::vec2(300, 0));
    mana_icon_ = Sprite::addSpriteChild(this, "assets/UI/Blue Potion.png", 0.5f, Anchor::CENTER_LEFT);
    mana_icon_->setOffset(mana_icon_->getOffset() + glm::vec2(270, 0));
    
}

void HUDStats::update(float dt)
{
    ObjectWorld::update(dt);
    // 获取Actor血量百分比，然后设置对应sprite
    update_health_bar();
    update_mana_bar();
}

void HUDStats::update_health_bar()
{
    if (!target_ || !health_bar_ || !target_->getStats()) return;
    health_bar_->setPercentage(glm::vec2(target_->getStats()->getHealth() / target_->getStats()->getMaxHealth(), 1.0f));
}

void HUDStats::update_mana_bar()
{
    if (!target_ || !mana_bar_ || !target_->getStats()) return;
    mana_bar_->setPercentage(glm::vec2(target_->getStats()->getMana() / target_->getStats()->getMaxMana(), 1.0f));
}
