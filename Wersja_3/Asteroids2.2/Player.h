#pragma once
#ifndef PLAYER_H
#define PLAYER_H

#include "Entity.h"

class player : public Entity
{
public:
    bool thrust;
	float speed = 4.0f;
    player();
    void update();
};

#endif
