#include "object.h"

void Object::handleEvents(SDL_Event &event)
{
    for(auto& child : children_){
       if (child->is_active_) child->handleEvents(event);
    }
}

void Object::update(float dt)
{
    if (!objects_to_add.empty()) {
        for (auto obj : objects_to_add) {
            addChild(obj);
        }
        objects_to_add.clear();
    }
   
    for (auto it = children_.begin();it != children_.end();) {
        auto child = *it;
        if (child->getNeedRemove()) {
            it = children_.erase(it);
            child->clean();
            delete child;
        }
        else {
            if (child->getActive()) child->update(dt);
            it++;
        }
    }
}

void Object::render()
{
    for(auto& child : children_){
        if (child->is_active_) child->render();
    }
}

void Object::clean()
{
    for(auto& child : children_){
        if (child->is_active_) child->clean();
    }
    children_.clear();
}
