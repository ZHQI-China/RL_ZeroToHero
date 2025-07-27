#pragma once
#include <string>
#include "core/object/object_affiliate.h"
#include "global/defs.h"

class Sprite:public ObjectAffiliate{
protected:
    Texture texture_;
    bool is_finish_ = false;
    glm::vec2 percentage_ = glm::vec2(1.0f);       // 决定图片原始区域的百分比
public:
    Sprite() = default;
    ~Sprite() = default;
public:
    static Sprite* addSpriteChild(ObjectWorld* parent, const std::string& file_path, float scale = 1.0f, Anchor anchor = Anchor::CENTER);

    // getter and setter
    Texture& getTexture() {return texture_;}
    virtual void setTexture(const Texture& texture) {texture_ = texture;size_ = {texture_.src_rect.w, texture_.src_rect.h};}
public:
    virtual void render() override;
    bool getFinish() const { return is_finish_; }
    void setFinish(bool is_finish) { is_finish_ = is_finish; }
	glm::vec2 getPercentage() const { return percentage_; }
	void setPercentage(const glm::vec2& percentage) { percentage_ = percentage; }
};