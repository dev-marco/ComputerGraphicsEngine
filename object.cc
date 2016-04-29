#include "object.h"

std::unordered_set<const Object *> Object::invalid{ NULL };
std::stack<Object *> Object::marked{};
