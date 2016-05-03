#ifndef SRC_ENGINE_OBJECT_H_
#define SRC_ENGINE_OBJECT_H_

#include <memory>
#include <array>
#include <stack>
#include <unordered_map>
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
    std::unordered_set<Object *> children, tested_collisions;
    Object *parent = nullptr;
    std::valarray<double> position, speed, acceleration;
    const Shader::Program *shader = nullptr;

    static void delayedDestroy(void);

public:

    inline static double vectorSize (const std::valarray<double> &vector) {
        return std::sqrt((vector * vector).sum());
    }

    inline static std::valarray<double> resizeVector (const std::valarray<double> &vector, double vector_size, double new_size) {
        return vector * (new_size / vector_size);
    }

    inline static std::valarray<double> resizeVector (const std::valarray<double> &vector, double new_size) {
        return Object::resizeVector(vector, Object::vectorSize(vector), new_size);
    }

    inline static bool isValid (const Object *obj) { return Object::invalid.find(obj) == Object::invalid.end(); }

    inline Object (
        const std::array<double, 3> &_position = { 0.0, 0.0, 0.0 },
        bool _display = true,
        Mesh *_mesh = new Mesh(),
        Mesh *_collider = nullptr,
        Background *_background = new Background(),
        const std::array<double, 3> &_speed = { 0.0, 0.0, 0.0 },
        const std::array<double, 3> &_acceleration = { 0.0, 0.0, 0.0 }
    ) : display(_display), mesh(_mesh), collider(_collider), background(_background), position(_position.data(), 3), speed(_speed.data(), 3), acceleration(_acceleration.data(), 3) {
        Object::invalid.erase(this);
    };

    inline virtual ~Object () { Object::invalid.insert(this); }

    inline bool detectCollision (const Object *other, std::valarray<double> &point) const {
        return this->getCollider()->detectCollision(other->getCollider(), this->getPosition(), other->getPosition(), point);
    }

    inline bool collides () const { return this->collider != nullptr; }

    inline bool isMoving (void) const { for (double s : this->speed) { if (s != 0) { return true; } } return false; }

    inline void addChild (Object *obj) { if (Object::isValid(this) && Object::isValid(obj)) { obj->parent = this, this->children.insert(obj); } }
    inline void removeChild (Object *obj) { if (Object::isValid(this) && Object::isValid(obj)) { obj->parent = nullptr, this->children.erase(obj); } }

    inline void setParent (Object *obj) { if (Object::isValid(this) && Object::isValid(obj)) { this->parent->addChild(obj); } }
    inline void removeParent (void) { if (Object::isValid(this) && Object::isValid(this->parent)) { this->parent->removeChild(this); } }
    inline Object *getParent (void) const { return this->parent; }

    inline const std::unordered_set<Object *> &getChildren (void) const { return this->children; }

    inline bool hasTestedCollision (Object *other) const { return this->tested_collisions.find(other) != this->tested_collisions.end(); }

    void move(double delta_time, bool collision_detect);
    void update(double now, double delta_time, unsigned tick, bool collision_detect);
    void draw(double ratio = 1.0) const;

    inline const Shader::Program *getShader (void) const { return this->shader; }

    inline void destroy (void) { this->display = false, Object::marked.push(this); }

    inline void setShader (const Shader::Program *program) { this->shader = program; }

    inline std::valarray<double> getPosition (void) const { return this->position; }
    inline std::valarray<double> getSpeed (void) const { return this->speed; }
    inline std::valarray<double> getAcceleration (void) const { return this->acceleration; }

    inline void setPosition (const std::valarray<double> &_position) { this->position = _position; }
    inline void setSpeed (const std::valarray<double> &_speed) { this->speed = _speed; }
    inline void setAcceleration (const std::valarray<double> &_acceleration) { this->acceleration = _acceleration; }

    inline Mesh *getMesh (void) const { return this->mesh; }
    inline Mesh *getCollider (void) const { return this->collider; }

    inline operator bool () const { return Object::isValid(this); }

    virtual inline void onCollision (const Object *other, const std::valarray<double> &point) {}
    virtual inline void beforeDestroy () {}
    virtual inline void afterDestroy () {}
    virtual inline void beforeUpdate (double now, double delta_time, unsigned tick) {}
    virtual inline void afterUpdate (double now, double delta_time, unsigned tick) {}
    virtual inline void beforeDraw () const {}
    virtual inline void afterDraw () const {}

    virtual inline std::string getType () const { return "object"; }

};

#endif
