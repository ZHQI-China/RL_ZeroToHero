#pragma once
#include "object_screen.h"
#include "object_world.h"

class ObjectAffiliate:public Object {
protected:
    ObjectWorld* parent_{nullptr};         // 父对象
    glm::vec2 offset_{0, 0};                // 偏移量
    glm::vec2 size_{0, 0};                  // 大小
    Anchor anchor_ = Anchor::CENTER;        // 锚点
public:
    ObjectAffiliate()=default;
    ObjectAffiliate(ObjectWorld* parent):parent_(parent) {}
    virtual ~ObjectAffiliate() {}

public:
    void setOffsetByAnchor(Anchor anchor);

    // setter and getter
    void setParent(ObjectWorld* parent) { parent_ = parent; }
    ObjectWorld* getParent() const{ return parent_; }
    void setOffset(glm::vec2 offset) { offset_ = offset; }
    glm::vec2 getOffset() const{ return offset_; }

    // 设置大小后，根据锚点调整offset
    void setSize(glm::vec2 size) { size_ = size; setOffsetByAnchor(anchor_);}

    glm::vec2 getSize() const{ return size_; }
    void setScale(float scale) { setSize(size_*scale); }//在原基础上直接*scale-->todo:新增scale_
    Anchor getAnchor() const { return anchor_; }
    void setAnchor(Anchor anchor) { anchor_ = anchor; }
};