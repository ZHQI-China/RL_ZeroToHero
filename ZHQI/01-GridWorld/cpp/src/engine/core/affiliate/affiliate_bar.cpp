#include "affiliate_bar.h"
#include "util/renderer.h"
AffiliateBar *AffiliateBar::addAffiliateBarChild(ObjectWorld*parrent, glm::vec2 size, Anchor anchor)
{
    auto bar = new AffiliateBar();
    bar->init();
    bar->setAnchor(anchor);
    
    bar->setSize(size);
    if (parrent){
        bar->setParent(parrent);
        parrent->addChild(bar);
    }
    return bar;
}

void AffiliateBar::render()
{
    auto pos = parent_->getScreenPos() + offset_;
    if (percentage_ > 0.7f){
        Renderer::renderHBar(pos, size_, percentage_, color_high_);
    } else if (percentage_ > 0.3f){
        Renderer::renderHBar(pos, size_, percentage_, color_mid_);
    } else {
        Renderer::renderHBar(pos, size_, percentage_, color_low_);
    }
}
