#include "sprite.h"
#include "util/asset_store.h"
#include "util/renderer.h"

Sprite* Sprite::addSpriteChild(ObjectWorld* parent, const std::string& file_path, float scale, Anchor anchor)
{
    auto sprite = new Sprite();
    sprite->init();
    sprite->setParent(parent);
    sprite->setTexture(Texture(file_path));
    sprite->setAnchor(anchor);
    sprite->setScale(scale);
    parent->addChild(sprite);
    return sprite;
}
void Sprite::render()
{
    if(!texture_.texture) return;
    if(!parent_) return;
    auto pos = parent_->getScreenPos()+offset_;
    Renderer::renderTexture(texture_, pos, size_, percentage_);
}
