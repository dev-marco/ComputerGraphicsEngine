#include "object.h"

namespace Engine {

    std::unordered_set<const Object *> Object::invalid{ nullptr };
    std::set<Object *> Object::marked{};

    void Object::delayedDestroy (void) {

        std::set<Object *> swapper;

        while (!Object::marked.empty()) {

            swapper.clear();
            Object::marked.swap(swapper);

            for (auto obj : swapper) {
                if (Object::isValid(obj, false)) {

                    obj->beforeDestroy();

                    obj->removeParent();

                    for (const auto &child : obj->children) {
                        child->destroy();
                    }

                    obj->children.clear();

                    Object::invalid.insert(obj);

                    obj->afterDestroy();

                    delete obj;
                }
            }
        }
    }

    void Object::move (float_max_t delta_time, bool collision_detect) {

        if (collision_detect) {

            static std::list<Object *> moving;

            for (auto &child : this->children) {
                if (child->isMoving()) {
                    if (child->collides()) {
                        moving.push_back(child);
                    } else {
                        child->setPosition(child->getPosition() + child->getSpeed() * delta_time);
                    }
                }
            }

            if (!moving.empty()) {

                constexpr unsigned collision_samples = 4;
                const float_max_t multiplier = delta_time / static_cast<float_max_t>(collision_samples);
                Vec<3> point;
                static std::unordered_map<const Object *, std::unordered_set<const Object *>> collided;

                for (unsigned i = 0; i < collision_samples; ++i) {

                    for (auto child_it = moving.begin(), child_end = moving.end(); child_it != child_end; ) {

                        auto &child = *child_it;
                        bool valid = Object::isValid(child) && child->collides();

                        if (valid) {

                            const Vec<3> delta_speed = child->getSpeed() * multiplier;

                            for (auto &other : this->children) {

                                if (
                                    child != other &&
                                    Object::isValid(other) &&
                                    other->collides() &&
                                    !collided[child].count(other) &&
                                    child->detectCollision(other, delta_speed, other->getSpeed() * multiplier, point)
                                ) {
                                    child->onCollision(other, point);

                                    if (Object::isValid(other)) {
                                        other->onCollision(child, point);
                                    }

                                    if (!(Object::isValid(child) && child->collides())) {
                                        valid = false;
                                        collided[child].clear();
                                        break;
                                    } else {
                                        collided[other].insert(child);
                                    }
                                }
                            }
                        }

                        if (valid) {
                            child_it++;
                        } else {
                            child->setPosition(child->getPosition() + child->getSpeed() * static_cast<float_max_t>(collision_samples - i) * multiplier);
                            child_it = moving.erase(child_it);
                        }
                    }

                    for (auto &child : moving) {
                        child->setPosition(child->getPosition() + child->getSpeed() * multiplier);
                    }

                    collided.clear();
                }
            }

            moving.clear();
        } else {
            for (auto &child : this->children) {
                child->setPosition(child->getPosition() + child->getSpeed() * delta_time);
            }
        }
    }

    void Object::update (float_max_t now, float_max_t delta_time, unsigned tick, bool collision_detect) {

        static bool destroy_shared = true;
        const bool destroy_local = destroy_shared;

        destroy_shared = false;

        if (Object::isValid(this)) {

            this->beforeUpdate(now, delta_time, tick);

            this->move(delta_time, collision_detect);
            this->setSpeed(this->getSpeed() + this->acceleration * delta_time);

            for (auto &child : this->children) {
                child->update(now, delta_time, tick, collision_detect);
            }

            this->afterUpdate(now, delta_time, tick);
        }

        if (destroy_local) {
            destroy_shared = true;
            Object::delayedDestroy();
        }
    }

    void Object::alwaysUpdate (float_max_t now, float_max_t delta_time, unsigned tick, bool collision_detect) {

        if (Object::isValid(this)) {

            this->beforeAlwaysUpdate(now, delta_time, tick);

            for (auto &child : this->children) {
                child->alwaysUpdate(now, delta_time, tick, collision_detect);
            }

            this->afterAlwaysUpdate(now, delta_time, tick);
        }

    }

    void Object::draw (bool only_border) const {

        if (Object::isValid(this)) {
            if (this->display) {

                Mesh *mesh = this->getMesh();

                Draw::push();

                Draw::translate(this->getPosition());
                Draw::rotate(this->getOrientation());

                // Shader::push(this->shader);

                this->beforeDraw(only_border);

                if (mesh) {
                    mesh->draw(only_border);
                }

                for (const auto &child : this->getChildren()) {
                    child->draw(only_border);
                }

                this->afterDraw(only_border);

                // Shader::pop();

                Draw::pop();
            }
        }
    }

    void Object::debugInfo (std::ostream &out, const std::string shift) const {
        if (Object::isValid(this)) {

            Engine::Mesh *mesh = this->getMesh();
            std::string next_shift = shift + ' ';

            out << shift << "Type: " << this->getType() << std::endl;
            out << shift << "Position: " << this->getPosition() << std::endl;

            if (mesh) {
                out << shift << "Mesh:" << std::endl;
                mesh->debugInfo(out, next_shift);
            }

            out << shift << "Speed: " << this->getSpeed() << std::endl;
            if (!this->getChildren().empty()) {
                out << shift << "Children:" << std::endl;
                for (auto &child : this->getChildren()) {
                    child->debugInfo(out, next_shift);
                }
            }
            out << std::endl;
        }
    }
};
