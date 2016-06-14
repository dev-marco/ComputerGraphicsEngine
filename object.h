#ifndef SRC_ENGINE_OBJECT_H_
#define SRC_ENGINE_OBJECT_H_

#include <memory>
#include <array>
#include <stack>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include "defaults.h"
#include "shader.h"
#include "vec.h"
#include "quaternion.h"
#include "mesh.h"

namespace Engine {
    class Object {

        static std::unordered_set<const Object *> invalid;
        static std::set<Object *> marked;

        bool display = true;
        Mesh *mesh = nullptr, *collider = nullptr;
        std::list<Object *> children;
        Object *parent = nullptr;
        Shader::Program *shader = nullptr;
        float_max_t
            mass = 1.0,
            min_speed = 0.0,
            max_speed = std::numeric_limits<float_max_t>::infinity(),
            min_acceleration = 0.0,
            max_acceleration = std::numeric_limits<float_max_t>::infinity(),
            max_force = std::numeric_limits<float_max_t>::infinity();
        Vec<3> position, speed, acceleration;
        Quaternion orientation;

        static void delayedDestroy(void);

    public:

        inline static bool isValid (const Object *obj, bool is_marked = true) {
            return Object::invalid.find(obj) == Object::invalid.end() || (is_marked && Object::marked.count(const_cast<Object *>(obj)));
        }

        inline Object (
            const Vec<3> &_position = Vec<3>::origin,
            const Quaternion &_orientation = Quaternion::identity,
            bool _display = true,
            Mesh *_mesh = nullptr,
            Mesh *_collider = nullptr,
            const Vec<3> &_speed = Vec<3>::zero,
            const Vec<3> &_acceleration = Vec<3>::zero,
            float_max_t _mass = 1.0,
            float_max_t _min_speed = 0.0,
            float_max_t _max_speed = std::numeric_limits<float_max_t>::infinity(),
            float_max_t _min_acceleration = 0.0,
            float_max_t _max_acceleration = std::numeric_limits<float_max_t>::infinity(),
            float_max_t _max_force = std::numeric_limits<float_max_t>::infinity()
        ) : display(_display), mass(_mass), min_speed(_min_speed), max_speed(_max_speed), max_force(_max_force), position(_position), orientation(_orientation) {
            this->setMesh(_mesh);
            this->setCollider(_collider);
            this->setAcceleration(_acceleration);
            this->setSpeed(_speed);
            Object::invalid.erase(this);
        };

        inline virtual ~Object (void) { Object::invalid.insert(this); }

        inline bool detectCollision (const Object *other, const Vec<3> &my_speed, const Vec<3> &other_speed, Vec<3> &point) const {
            return this->getCollider()->detectCollision(other->getCollider(), this->getPosition(), my_speed, other->getPosition(), other_speed, point);
        }

        inline bool collides (void) const { return this->collider != nullptr; }

        inline bool isMoving (void) const { return this->getSpeed(); }

        inline void addChild (Object *obj) {
            if (Object::isValid(this) && Object::isValid(obj)) {
                obj->parent = this;
                obj->onSetParent(this);
                this->children.push_back(obj);
                this->onAddChild(obj);
            }
        }
        inline void removeChild (Object *obj) {
            if (Object::isValid(this)) {
                if (Object::isValid(obj)) {
                    obj->parent = nullptr;
                    obj->onRemoveParent(this);
                }
                this->children.remove(obj);
                this->onRemoveChild(obj);
            }
        }

        inline void setParent (Object *obj) {
            if (Object::isValid(this) && Object::isValid(obj)) {
                this->parent->addChild(obj);
            }
        }
        inline void removeParent (void) {
            if (Object::isValid(this)) {
                if (Object::isValid(this->parent)) {
                    this->parent->removeChild(this);
                } else {
                    Object *parent = this->parent;
                    this->parent = nullptr;
                    this->onRemoveParent(parent);
                }
            }
        }
        inline Object *getParent (void) const { return this->parent; }

        inline const std::list<Object *> &getChildren (void) const { return this->children; }

        virtual void move(float_max_t delta_time, bool collision_detect) final;
        virtual void update(float_max_t now, float_max_t delta_time, unsigned tick, bool collision_detect) final;
        virtual void alwaysUpdate(float_max_t now, float_max_t delta_time, unsigned tick, bool collision_detect) final;
        virtual void draw(bool only_border = false) const final;

        inline Shader::Program *getShader (void) const { return this->shader; }

        inline virtual void destroy (void) final {
            if (Object::isValid(this)) {
                this->display = false;
                this->collider = nullptr;
                Object::marked.insert(this);
            }
        }

        inline void setShader (Shader::Program *program) { this->shader = program; }

        inline float_max_t getMinSpeed (void) const { return this->min_speed; }
        inline float_max_t getMaxSpeed (void) const { return this->max_speed; }
        inline float_max_t getMinAcceleration (void) const { return this->min_acceleration; }
        inline float_max_t getMaxAcceleration (void) const { return this->max_acceleration; }

        inline void setMinSpeed (float_max_t _min_speed) { this->min_speed = _min_speed; this->setSpeed(this->getSpeed()); }
        inline void setMaxSpeed (float_max_t _max_speed) { this->max_speed = _max_speed; this->setSpeed(this->getSpeed()); }
        inline void setMinAcceleration (float_max_t _min_acceleration) { this->min_acceleration = _min_acceleration; this->setAcceleration(this->getAcceleration()); }
        inline void setMaxAcceleration (float_max_t _max_acceleration) { this->max_acceleration = _max_acceleration; this->setAcceleration(this->getAcceleration()); }

        inline float_max_t getMaxForce (void) const { return this->max_force; }
        inline void setMaxForce (float_max_t _max_force) { this->max_force = _max_force; }

        inline const Vec<3> &getPosition (void) const { return this->position; }
        inline const Quaternion &getOrientation (void) const { return this->orientation; }
        inline const Vec<3> &getSpeed (void) const { return this->speed; }
        inline const Vec<3> &getAcceleration (void) const { return this->acceleration; }
        inline float_max_t getMass (void) const { return this->mass; }

        inline void setPosition (const Vec<3> &_position) { this->position = _position; }
        inline void setOrientation (const Quaternion &_orientation) { this->orientation = _orientation; }
        inline void setSpeed (const Vec<3> &_speed) { this->speed = _speed.clamped(this->getMinSpeed(), this->getMaxSpeed()); }
        inline void setAcceleration (const Vec<3> &_acceleration) { this->acceleration = _acceleration.clamped(this->getMinAcceleration(), this->getMaxAcceleration()); }
        inline void setMass (float_max_t _mass) { this->mass = _mass; }

        inline void applyForce (const Vec<3> &_force) { this->setAcceleration(this->getAcceleration() + (_force / this->getMass()).clamped(0.0, this->getMaxForce())); }

        inline Mesh *getMesh (void) const { return this->mesh; }
        inline Mesh *getCollider (void) const { return this->collider; }

        inline void setMesh (Mesh *_mesh) { this->mesh = _mesh; }
        inline void setCollider (Mesh *_collider) { this->collider = _collider; }

        inline operator bool () const { return Object::isValid(this); }

        virtual inline void onCollision (const Object *other, const Vec<3> &point) {}
        virtual inline void beforeDestroy () {}
        virtual inline void afterDestroy () {}
        virtual inline void beforeUpdate (float_max_t now, float_max_t delta_time, unsigned tick) {}
        virtual inline void afterUpdate (float_max_t now, float_max_t delta_time, unsigned tick) {}
        virtual inline void beforeAlwaysUpdate (float_max_t now, float_max_t delta_time, unsigned tick) {}
        virtual inline void afterAlwaysUpdate (float_max_t now, float_max_t delta_time, unsigned tick) {}
        virtual inline void beforeDraw (bool only_border) const {}
        virtual inline void afterDraw (bool only_border) const {}
        virtual inline void onAddChild (Object *child) {}
        virtual inline void onRemoveChild (Object *child) {}
        virtual inline void onSetParent (Object *parent) {}
        virtual inline void onRemoveParent (Object *parent) {}

        virtual void debugInfo (std::ostream &out, const std::string shift = "") const;

        virtual inline std::string getType () const { return "object"; }

    };
};

#endif
