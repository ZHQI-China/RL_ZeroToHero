#pragma once
#include "core/object/object_world.h"
#include <glm/glm.hpp>
#include <vector>
#include <string>

class Scene: public Object {

    friend class Engine;

protected:
    glm::vec2 camera_position_{};  // 相机位置
    glm::vec2 world_size_{};       // 世界大小

    std::string name_;

    std::vector<ObjectWorld*> children_world_;  // 场景中的物体
    std::vector<ObjectScreen*> children_screen_;// 屏幕中的物体

public:
    Scene() = default;
    Scene(const std::string& name) :name_(name) {}
    ~Scene() = default;

    virtual void init() override;
    virtual void handleEvents(SDL_Event& event) override;
    virtual void update(float dt) override;
    virtual void render() override;
    virtual void clean() override;

    glm::vec2 screenToWorld(const glm::vec2& screen_position) const {return screen_position +camera_position_;}
    glm::vec2 worldToScreen(const glm::vec2& world_position) const {return world_position -camera_position_;}
    glm::vec2 getCameraPosition() const {return camera_position_;}
    void setCameraPosition(const glm::vec2& position) {
        camera_position_ = position;
        camera_position_ = glm::clamp(camera_position_, glm::vec2(-30.f), world_size_-Engine::getInstance().getScreenSize()+glm::vec2(30.f));
    }

    void setWorldSize(const glm::vec2& size) {world_size_ = size;}
    glm::vec2 getWorldSize() const {return world_size_;}
    std::vector<ObjectScreen*>& getChildrenScreen() { return children_screen_; }
    std::vector<ObjectWorld*>& getChildrenWorld() { return children_world_; }

    virtual void addChild(Object* child) override;
    virtual void removeChild(Object* child) override;
};