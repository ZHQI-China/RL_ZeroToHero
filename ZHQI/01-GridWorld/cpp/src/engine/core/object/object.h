#pragma once
#include <vector>
#include "global/engine.h"
#include "global/defs.h"

class  Object
{
protected:
    ObjectType type_ = ObjectType::None;
    //Engine& engine_ = Engine::getInstance();
    std::vector<Object*> children_;
    std::vector<Object*> objects_to_add;

    bool is_active_ = true;
    bool need_remove_ = false;
public:
    // 所有的类，构造函数和析构函数均为空
    Object() = default;
    ~Object() = default;

    virtual void init(){};
    virtual void handleEvents(SDL_Event& event);
    virtual void update(float dt);
    virtual void render();
    virtual void clean();

    //virtual Object* clone() { return nullptr; }

public:
    // 用于初始化时添加
    virtual void addChild(Object* child){ children_.push_back(child); }

    // 运行时添加
    virtual void safeAddChild(Object* child){ objects_to_add.push_back(child); }

    virtual void removeChild(Object* child){ children_.erase(std::remove(children_.begin(), children_.end(), child), children_.end()); }

    // setter and getter
    ObjectType getType()const{ return type_; }
    void setType(ObjectType type){ type_ = type; }
    void setActive(bool flag) { is_active_ = flag; }
    bool getActive() const{ return is_active_; }
    bool getNeedRemove() const { return need_remove_; }
    void setNeedRemove(bool need_remove) { need_remove_ = need_remove; }
};
