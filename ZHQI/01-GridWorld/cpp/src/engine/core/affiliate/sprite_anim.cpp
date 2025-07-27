#include "sprite_anim.h"

SpriteAnim* SpriteAnim::addSpriteAnimChild(ObjectWorld* parent, const std::string& file_path, float scale, Anchor anchor)
{
    auto sprite_anim = new SpriteAnim();
    sprite_anim->init();
    sprite_anim->setParent(parent);
    sprite_anim->setTexture(Texture(file_path));
    sprite_anim->setAnchor(anchor);
    sprite_anim->setScale(scale);
    parent->addChild(sprite_anim);
    return sprite_anim;
}

void SpriteAnim::update(float dt)
{
    frame_timer_ += dt;
    if (frame_timer_ >= 1.0f / fps_)
    {
        current_frame_++;
        if (current_frame_ >= total_frames_)
        {
            if(is_loop_) current_frame_ = 0;
            else is_finish_ = true;
        }
        frame_timer_ = 0.0f;
    }
    texture_.src_rect.x = texture_.src_rect.w * current_frame_;
}

void SpriteAnim::setTexture(const Texture& texture)
{
    texture_ = texture;
    total_frames_ = static_cast<int>(texture.src_rect.w / texture.src_rect.h);
    texture_.src_rect.w = texture.src_rect.h;
    size_ = glm::vec2(texture_.src_rect.w, texture_.src_rect.h);
}
