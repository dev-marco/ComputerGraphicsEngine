#include "object.h"

namespace Engine {
    std::unordered_set<const Object *> Object::invalid{ NULL };
    std::stack<Object *> Object::marked{};

    void Object::delayedDestroy (void ) {

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

    void Object::move (double delta_time, bool collision_detect) {

        if (collision_detect) {

            static std::unordered_set<Object *> moving;

            for (auto &child : this->children) {
                if (child->isMoving()) {
                    if (child->collides()) {
                        moving.insert(child);
                    } else {
                        child->setPosition(child->getSpeed() * delta_time);
                    }
                }
            }

            if (!moving.empty()) {

                constexpr unsigned collision_samples = 16;
                const double multiplier = delta_time / static_cast<double>(collision_samples);
                std::valarray<double> point;

                for (unsigned i = 0; i < collision_samples; ++i) {

                    static std::unordered_map<Object *, std::unordered_set<Object *>> collided;

                    for (auto &child : moving) {
                        child->setPosition(child->getPosition() + child->getSpeed() * multiplier);
                    }

                    for (auto &child : std::unordered_set<Object *>(moving)) {
                        for (auto &other : this->children) {
                            if (child != other && !collided[child].count(other) && child->detectCollision(other, point)) {
                                child->onCollision(other, point);
                                other->onCollision(child, point);
                                collided[other].insert(child);
                            }
                        }
                    }

                    collided.clear();
                }
            }

            moving.clear();
        } else {
            for (auto &child : this->children) {
                child->setPosition(child->getSpeed() * delta_time);
            }
        }
    }

    void Object::update (double now, double delta_time, unsigned tick, bool collision_detect) {

        static bool destroy_shared = true;
        bool destroy_local = destroy_shared;

        destroy_shared = false;

        if (Object::isValid(this)) {

            this->beforeUpdate(now, delta_time, tick);

            this->move(delta_time, collision_detect);
            this->speed += this->acceleration * delta_time;

            for (const auto &child : this->children) {
                child->update(now, delta_time, tick, collision_detect);
            }

            this->afterUpdate(now, delta_time, tick);
        }

        if (destroy_local) {
            destroy_shared = true;
            Object::delayedDestroy();
        }
    }

    void Object::draw (double ratio, bool only_border) const {

        if (Object::isValid(this)) {
            if (this->display) {

                this->beforeDraw(ratio, only_border);

                // Shader::push(this->shader);

                this->mesh->draw(this->position, this->background, only_border, ratio);

                for (const auto &child : this->children) {
                    child->draw(ratio, only_border);
                }

                // Shader::pop();

                this->afterDraw(ratio, only_border);
            }
        } else {
            std::cout << "drawing error" << std::endl;
        }
    }
};
