#include "object.h"

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

void Object::move (bool collision_detect) {

    if (collision_detect && this->isMoving()) {

        Object *parent = this->getParent();

        if (parent && this->collides()) {

            const unsigned collision_samples = 8;
            std::valarray<double> start_speed = this->getSpeed(), delta_speed = start_speed / static_cast<double>(collision_samples);

            for (unsigned i = 0; i < collision_samples && (this->getSpeed() == start_speed).min(); i++) {

                double multiplier = static_cast<double>(i) / static_cast<double>(collision_samples);
                this->position += delta_speed;

                for (const auto &other : parent->getChildren()) {

                    if (this != other && other->collides() && !this->hasTestedCollision(other)) {

                        std::valarray<double> other_position = other->getPosition() + other->getSpeed() * multiplier;

                        other->tested_collisions.insert(this);

                        if (this->detectCollision(other, this->position, other_position)) {
                            this->onCollision(other);
                            other->onCollision(this);
                            this->tested_collisions.insert(other);
                            std::cout << this->getType() << " " << other->getType() << std::endl;
                        }
                    }
                }
            }

            this->tested_collisions.clear();

        } else {
            this->position += this->speed;
        }
    }
}

void Object::update (double now, unsigned tick, bool collision_detect) {

    static bool destroy_shared = true;
    bool destroy_local = destroy_shared;

    destroy_shared = false;

    if (Object::isValid(this)) {

        this->beforeUpdate(now, tick);

        this->move(collision_detect);
        this->speed += this->acceleration;

        for (const auto &child : this->children) {
            child->update(now, tick, collision_detect);
        }

        this->afterUpdate(now, tick);
    }

    if (destroy_local) {
        destroy_shared = true;
        Object::delayedDestroy();
    }
}

void Object::draw (double ratio) const {

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
