#include "Entity.h"
#include <SFML/Graphics.hpp>

Entity::Entity() : life(true) {}

void Entity::settings(Animation& a, int X, int Y, float Angle, int radius)
{
    anim = a;
    x = X; y = Y;
    angle = Angle;
    R = radius;
}

void Entity::update() {}

void Entity::draw(sf::RenderWindow& app)
{
    anim.sprite.setPosition(x, y);
    anim.sprite.setRotation(angle + 90);
    app.draw(anim.sprite);

    sf::CircleShape circle(R);
    circle.setFillColor(sf::Color(255, 0, 0, 170));
    circle.setPosition(x, y);
    circle.setOrigin(R, R);
}

Entity::~Entity() {}
