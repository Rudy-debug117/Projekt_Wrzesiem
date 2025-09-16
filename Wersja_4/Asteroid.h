#pragma once
#ifndef ASTEROID_H
#define ASTEROID_H

#include "Entity.h"

class asteroid : public Entity
{
public:
    asteroid();
    void update();
};

#endif
