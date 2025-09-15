#include "Bullet.h"
#include "Constants.h"
#include <cmath>

bullet::bullet()
{
    name = "bullet";
}

void bullet::update()
{
    dx = std::cos(angle * DEGTORAD) * 6;
    dy = std::sin(angle * DEGTORAD) * 6;
    x += dx;
    y += dy;

    if (x > W || x < 0 || y > H || y < 0) life = false;
}
