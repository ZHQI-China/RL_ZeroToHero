#include "object_affiliate.h"

void ObjectAffiliate::setOffsetByAnchor(Anchor anchor)
{
    anchor_ = anchor;
    if (anchor != Anchor::NONE) {
        offset_ = glm::vec2(-AnchorOffsets[static_cast<int>(anchor)][0] * size_.x / 2.0f, -AnchorOffsets[static_cast<int>(anchor)][1] * size_.y / 2.0f);
    }
}
