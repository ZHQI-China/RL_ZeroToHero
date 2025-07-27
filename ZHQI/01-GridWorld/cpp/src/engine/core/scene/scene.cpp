#include "scene.h"

void Scene::init()
{
    Object::init();
    for (auto &child : children_screen_)
        if (child->getActive()) child->init();
    for (auto &child : children_world_)
        if (child->getActive()) child->init();
}

void Scene::handleEvents(SDL_Event &event)
{
    Object::handleEvents(event);
    for (auto &child : children_screen_)
        if (child->getActive()) child->handleEvents(event);
    for (auto &child : children_world_)
        if (child->getActive()) child->handleEvents(event);
}

void Scene::update(float dt)
{
    Object::update(dt);
    for (auto it = children_world_.begin();it != children_world_.end();) {
        auto child = *it;
        if (child->getNeedRemove()) {
            it = children_world_.erase(it);
            child->clean();
            delete child;
        }
        else {
            if (child->getActive()) child->update(dt);
            it++;
        }
    }
    for (auto it = children_screen_.begin();it != children_screen_.end();) {
        auto child = *it;
        if (child->getNeedRemove()) {
            it = children_screen_.erase(it);
            child->clean();
            delete child;
        }
        else {
            if (child->getActive()) child->update(dt);
            it++;
        }
    }
}

void Scene::render()
{
    Object::render();
    for (auto &child : children_world_)
        if (child->getActive()) child->render();
    for (auto &child : children_screen_)
        if (child->getActive()) child->render();
}

void Scene::clean()
{
    Object::clean();
    for (auto &child : children_screen_)
        child->clean();
    for (auto &child : children_world_)
        child->clean();
    children_screen_.clear();
    children_world_.clear();
}

void Scene::addChild(Object *child)
{
    switch(child->getType())
    {
        case ObjectType::Screen:
            children_screen_.push_back(dynamic_cast<ObjectScreen*>(child));
            break;
        case ObjectType::World:
        case ObjectType::Enemy:
            children_world_.push_back(dynamic_cast<ObjectWorld*>(child));
            //SDL_Log("children_world_.push_back(dynamic_cast<ObjectWorld*>(child));");
            break;
        default:
            children_.push_back(child);
            break;
    }
}

void Scene::removeChild(Object *child)
{
    switch (child->getType())
    {
        case ObjectType::Screen:
            children_screen_.erase(std::remove(children_screen_.begin(), children_screen_.end(), dynamic_cast<ObjectScreen*>(child)), children_screen_.end());
            break;
        case ObjectType::World:
        case ObjectType::Enemy:
            children_world_.erase(std::remove(children_world_.begin(), children_world_.end(), dynamic_cast<ObjectWorld*>(child)), children_world_.end());
            break;
        default:
            children_.erase(std::remove(children_.begin(), children_.end(), child), children_.end());
            break;
    }
}
