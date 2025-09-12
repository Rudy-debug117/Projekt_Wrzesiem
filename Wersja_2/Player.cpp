#include "Player.h"
#include "Header.h"
#include <cmath>

player::player()
{
    name = "player";
}

void player::update()
{
    if (thrust)
    {
        dx += cos(angle * DEGTORAD) * speed * 0.05f;
        dy += sin(angle * DEGTORAD) * speed * 0.05f;
    }

    else
    {
        dx *= 0.99;
        dy *= 0.99;
    }

    int maxSpeed = 15;
    float speed = std::sqrt(dx * dx + dy * dy);
    if (speed > maxSpeed)
    {
        dx *= maxSpeed / speed;
        dy *= maxSpeed / speed;
    }

    x += dx;
    y += dy;

    if (x > W) x = 0; if (x < 0) x = W;
    if (y > H) y = 0; if (y < 0) y = H;
}
