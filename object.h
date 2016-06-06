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
#include "background.h"
#include "vec.h"
#include "quaternion.h"
#include "mesh.h"

namespace Engine {
    class Object {

        static std::unordered_set<const Object *> invalid;
        static std::set<Object *> marked;

        bool display;
        Mesh *mesh, *collider = nullptr;
        Background *background;
        std::list<Object *> children;
        Object *parent = nullptr;
        Vec<3> position, speed, acceleration;
        Quaternion orientation;
        Shader::Program *shader = nullptr;

        static void delayedDestroy(void);

    public:

        inline static bool isValid (const Object *obj, bool is_marked = true) {
            return Object::invalid.find(obj) == Object::invalid.end() || (is_marked && Object::marked.count(const_cast<Object *>(obj)));
        }

        inline Object (
            const Vec<3> &_position = Vec<3>::zero,
            const Quaternion &_orientation = Quaternion::identity,
            bool _display = true,
            Mesh *_mesh = new Mesh(),
            Mesh *_collider = nullptr,
            Background *_background = new Background(),
            const Vec<3> &_speed = Vec<3>::zero,
            const Vec<3> &_acceleration = Vec<3>::zero
        ) : display(_display), mesh(_mesh), collider(_collider), background(_background), position(_position), speed(_speed), acceleration(_acceleration), orientation(_orientation) {
            Object::invalid.erase(this);
        };

        inline virtual ~Object (void) { Object::invalid.insert(this); }

        inline bool detectCollision (const Object *other, const Vec<3> &my_speed, const Vec<3> &other_speed, Vec<3> &point) const {
            return this->getCollider()->detectCollision(other->getCollider(), this->getPosition(), my_speed, other->getPosition(), other_speed, point);
        }

        inline bool collides (void) const { return this->collider != nullptr; }

        inline bool isMoving (void) const { return this->getSpeed(); }

        inline void addChild (Object *obj) { if (Object::isValid(this) && Object::isValid(obj)) { obj->parent = this, this->children.push_back(obj); } }
        inline void removeChild (Object *obj) { if (Object::isValid(this) && Object::isValid(obj)) { obj->parent = nullptr, this->children.remove(obj); } }

        inline void setParent (Object *obj) { if (Object::isValid(this) && Object::isValid(obj)) { this->parent->addChild(obj); } }
        inline void removeParent (void) { if (Object::isValid(this) && Object::isValid(this->parent)) { this->parent->removeChild(this); } }
        inline Object *getParent (void) const { return this->parent; }

        inline const std::list<Object *> &getChildren (void) const { return this->children; }

        void move(float_max_t delta_time, bool collision_detect);
        void update(float_max_t now, float_max_t delta_time, unsigned tick, bool collision_detect);
        void draw(bool only_border = false) const;

        inline Shader::Program *getShader (void) const { return this->shader; }

        inline void destroy (void) {
            if (Object::isValid(this)) {
                this->display = false;
                this->collider = nullptr;
                Object::marked.insert(this);
            }
        }

        inline void setShader (Shader::Program *program) { this->shader = program; }

        inline const Vec<3> &getPosition (void) const { return this->position; }
        inline const Quaternion &getOrientation (void) const { return this->orientation; }
        inline const Vec<3> &getSpeed (void) const { return this->speed; }
        inline const Vec<3> &getAcceleration (void) const { return this->acceleration; }

        inline void setPosition (const Vec<3> &_position) { this->position = _position; }
        inline void setOrientation (const Quaternion &_orientation) { this->orientation = _orientation; }
        inline void setSpeed (const Vec<3> &_speed) { this->speed = _speed; }
        inline void setAcceleration (const Vec<3> &_acceleration) { this->acceleration = _acceleration; }

        inline Mesh *getMesh (void) const { return this->mesh; }
        inline Mesh *getCollider (void) const { return this->collider; }

        inline Background *getBackground (void) const { return this->background; }

        inline operator bool () const { return Object::isValid(this); }

        virtual inline void onCollision (const Object *other, const Vec<3> &point) {}
        virtual inline void beforeDestroy () {}
        virtual inline void afterDestroy () {}
        virtual inline void beforeUpdate (float_max_t now, float_max_t delta_time, unsigned tick) {}
        virtual inline void afterUpdate (float_max_t now, float_max_t delta_time, unsigned tick) {}
        virtual inline void beforeDraw (bool only_border) const {}
        virtual inline void afterDraw (bool only_border) const {}

        virtual void debugInfo (std::ostream &out, const std::string shift = "") const;

        virtual inline std::string getType () const { return "object"; }

    };
};

#endif
