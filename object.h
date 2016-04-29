#ifndef SRC_OBJECT_H_
#define SRC_OBJECT_H_

#include <memory>
#include <array>
#include <stack>
#include <unordered_set>
#include <iostream>
#include <valarray>
#include "shader.h"
#include "background.h"
#include "mesh.h"

class Object {

    static std::unordered_set<const Object *> invalid;
    static std::stack<Object *> marked;

    bool display;
    Mesh *mesh, *collider = nullptr;
    Background *background;
    std::unordered_set<Object *> children;
    Object *parent = nullptr;
    std::valarray<double> position, speed, acceleration;
    const Shader::Program *shader = nullptr;

    static void delayedDestroy () {

        while (!Object::marked.empty()) {

            auto obj = Object::marked.top();

            Object::marked.pop();

            if (Object::isValid(obj)) {

                obj->beforeDestroy();

                obj->removeParent();

                for (const auto &child : obj->getChildren()) {
                    child->destroy();
                }

                obj->children.clear();

                delete obj->background;
                delete obj->mesh;

                Object::invalid.insert(obj);

                obj->afterDestroy();

                delete obj;
            }
        }
    }

public:

    inline static bool isValid (const Object *obj) {
        return Object::invalid.find(obj) == Object::invalid.end();
    }

    inline Object (
        const std::array<double, 3> &_position = {0.0, 0.0, 0.0},
        bool _display = true,
        Mesh *_mesh = new Mesh(),
        Mesh *_collider = nullptr,
        Background *_background = new Background(),
        const std::array<double, 3> &_speed = {0.0, 0.0, 0.0},
        const std::array<double, 3> &_acceleration = {0.0, 0.0, 0.0}
    ) : display(_display), mesh(_mesh), collider(_collider), background(_background), position(_position.data(), 3), speed(_speed.data(), 3), acceleration(_acceleration.data(), 3) {
        Object::invalid.erase(this);
    };

    void detectCollisions () {

        auto child = this->children.begin(), child_end = this->children.end();

        while (child != child_end) {
            if (Object::isValid(*child) && (*child)->collides()) {
                for (auto next = std::next(child); next != child_end; next++) {
                    if ((*child)->detectCollision(*next)) {
                        (*child)->onCollision(*next);
                        (*next)->onCollision(*child);
                    }
                }
            }
            child++;
        }
    }

    virtual ~Object () {
        Object::invalid.insert(this);
    }

    bool detectCollision (const Object *other) const {
        return this->getCollider()->detectCollision(other->getCollider(), this->getPosition(), other->getPosition());
    }

    bool collides () {
        return this->collider != nullptr;
    }

    inline void addChild (Object *obj) {
        if (Object::isValid(this) && Object::isValid(obj)) {
            obj->parent = this;
            this->children.insert(obj);
        }
    }

    inline void removeChild (Object *obj) {
        if (Object::isValid(this) && Object::isValid(obj)) {
            obj->parent = nullptr;
            this->children.erase(obj);
        }
    }

    inline void setParent (Object *obj) {
        if (Object::isValid(this) && Object::isValid(obj)) {
            this->parent->addChild(obj);
        }
    }

    inline void removeParent () {
        if (Object::isValid(this) && Object::isValid(this->parent)) {
            this->parent->removeChild(this);
        }
    }

    inline const std::unordered_set<Object *> &getChildren () const {
        return this->children;
    }

    virtual void update (double now, unsigned tick) {

        static bool destroy_shared = true;
        bool destroy_local = destroy_shared;

        destroy_shared = false;

        if (Object::isValid(this)) {

            auto children(this->children);

            this->beforeUpdate(now, tick);

            this->position += this->speed;
            this->speed += this->acceleration;

            for (const auto &child : children) {
                child->update(now, tick);
            }

            this->afterUpdate(now, tick);
        }

        if (destroy_local) {
            destroy_shared = true;
            Object::delayedDestroy();
        }
    }

    virtual void draw (double ratio = 1.0) const {
        if (Object::isValid(this)) {
            if (this->display) {

                this->beforeDraw();

                // Shader::push(this->shader);

                this->mesh->draw(this->position, this->background, ratio);

                for (const auto &child : this->children) {
                    child->draw(ratio);
                }

                // Shader::pop();

                this->afterDraw();
            }
        } else {
            std::cout << "drawing error" << std::endl;
        }
    }

    inline const Shader::Program *getShader (void) const {
        return this->shader;
    }

    void destroy (void) { this->display = false, Object::marked.push(this); }

    inline void setShader (const Shader::Program *program) { this->shader = program; }

    inline std::valarray<double> getPosition (void) const { return this->position; }
    inline std::valarray<double> getSpeed (void) const { return this->speed; }
    inline std::valarray<double> getAcceleration (void) const { return this->acceleration; }

    inline void setPosition (const std::valarray<double> &_position) { this->position = _position; }
    inline void setSpeed (const std::valarray<double> &_speed) { this->speed = _speed; }
    inline void setAcceleration (const std::valarray<double> &_acceleration) { this->acceleration = _acceleration; }

    inline const Mesh *getMesh (void) const { return this->mesh; }
    inline const Mesh *getCollider (void) const { return this->collider; }

    inline operator bool () const { return Object::isValid(this); }

    virtual void onCollision (const Object *other) {}
    virtual void beforeDestroy () {}
    virtual void afterDestroy () {}
    virtual void beforeUpdate (double now, unsigned tick) {}
    virtual void afterUpdate (double now, unsigned tick) {}
    virtual void beforeDraw () const {}
    virtual void afterDraw () const {}

    virtual std::string getType () const { return "object"; }

};

#endif
