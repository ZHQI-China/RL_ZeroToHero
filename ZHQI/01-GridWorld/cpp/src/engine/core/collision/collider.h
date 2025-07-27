#pragma once
#include <engine/core/object/object_affiliate.h>
#include <engine/core/object/object_world.h>
#include <engine/util/math.h>
#include <engine/util/renderer.h>

#include "gjk.h"

class  Collider : public ObjectAffiliate
{
protected:
	Shape* shape_ = nullptr;

public:
	virtual void clean()override { delete shape_; }
	virtual void render() override {
		ObjectAffiliate::render();
		shape_->draw(offset_+ parent_->getScreenPos());
	};

	static Collider* addColliderChild(ObjectWorld* parent, Shape* shape, Anchor anchor = Anchor::CENTER) {
		auto collider = new Collider;
		collider->init();
		collider->shape_ = shape;
		collider->setAnchor(anchor);
		collider->setOffsetByAnchor(anchor);
		if (parent)
		{
			collider->setParent(parent);
			parent->addChild(collider);
		}
		return collider;
	}
	bool isColliding(Collider* other) const {
		return other ? GJK(*shape_, *other->shape_, parent_->getWorldPos()+offset_, other->parent_->getWorldPos()+other->getOffset()) : false;
	}
	void setRotation(float angle) { shape_->setRotation(angle); }

	// 用于碰撞检测：世界坐标
	AABB getAABB() { return shape_->getAABB(offset_+ parent_->getWorldPos()); }
};