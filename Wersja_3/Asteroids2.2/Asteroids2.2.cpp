#include <SFML/Graphics.hpp>
#include <list>
#include <ctime>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <memory>
#include <iostream>
#include <cmath>
#include "Asteroid.h"
#include "Bullet.h"
#include "Player.h"
#include "Animation.h"
#include "Collision.h"
#include "Header.h"

using namespace std;

int main()
{
    srand(time(0));

    // --- parametry gracza i strzelania ---
    float playerFireRate = 0.4f;
    float playerSpeed = 2.0f;
    const float maxPlayerSpeed = 6.0f;
    float fireCooldown = 0.0f;

    sf::RenderWindow app(sf::VideoMode(W, H), "Asteroids!");
    app.setFramerateLimit(60);

    // --- tekstury ---
    sf::Texture t1, t2, t3, t4, t5, t6;
    if (!t1.loadFromFile("C:/projekcik/images/spaceship.png")) {
        cerr << "Błąd ładowania: spaceship.png" << endl;
        return 1;
    }
    if (!t2.loadFromFile("C:/projekcik/images/explosions/type_C.png")) {
        cerr << "Błąd ładowania: explosions/type_C.png" << endl;
        return 1;
    }
    if (!t3.loadFromFile("C:/projekcik/images/rock.png")) {
        cerr << "Błąd ładowania: rock.png" << endl;
        return 1;
    }
    if (!t4.loadFromFile("C:/projekcik/images/rock_small.png")) {
        cerr << "Błąd ładowania: rock_small.png" << endl;
        return 1;
    }
    if (!t5.loadFromFile("C:/projekcik/images/explosions/type_B.png")) {
        cerr << "Błąd ładowania: type_B.png" << endl;
        return 1;
    }
    if (!t6.loadFromFile("C:/projekcik/images/fire_red.png")) {
        cerr << "Błąd ładowania: fire_red.png" << endl;
        return 1;
    }

    // --- animacje ---
    Animation sRock(t3, 0, 0, 64, 64, 16, 0.2);
    Animation sRock_small(t4, 0, 0, 64, 64, 16, 0.2);
    Animation sPlayer(t1, 40, 0, 40, 40, 1, 0);
    Animation sPlayer_go(t1, 40, 40, 40, 40, 1, 0);
    Animation sExplosion_ship(t5, 0, 0, 192, 192, 64, 0.5);
    Animation sExplosion(t2, 0, 0, 256, 256, 48, 0.5);
    Animation sBullet(t6, 0, 0, 32, 64, 16, 0.8);

    list<unique_ptr<Entity>> entities;

    // --- gracz ---
    auto playerPtr = make_unique<player>();
    playerPtr->settings(sPlayer, W / 2, H / 2, 0, 20);
    player* p = playerPtr.get();
    p->speed = playerSpeed;
    entities.push_back(std::move(playerPtr));

    float safeTime = 5.0f;
    sf::Clock clock;

    int destroyedAsteroids = 0;

    while (app.isOpen())
    {
        sf::Event event;
        while (app.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                app.close();

            // --- strzelanie (spacja) ---
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space)
            {
                if (fireCooldown <= 0.0f) {
                    auto bulletPtr = make_unique<bullet>();
                    bulletPtr->settings(sBullet, p->x, p->y, p->angle, 10);
                    entities.push_back(std::move(bulletPtr));
                    fireCooldown = playerFireRate;
                }
            }
        }

        // --- cooldown strzału ---
        if (fireCooldown > 0.0f)
            fireCooldown -= 1.0f / 60.0f;

        // --- sterowanie graczem ---
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) p->angle += 3;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) p->angle -= 3;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) p->thrust = true;
        else p->thrust = false;

        // --- spawn asteroid ---
        if (clock.getElapsedTime().asSeconds() > safeTime)
        {
            if (rand() % 150 == 0)
            {
                auto asteroidPtr = make_unique<asteroid>();
                asteroidPtr->settings(sRock, 0, rand() % H, rand() % 360, 25);
                entities.push_back(std::move(asteroidPtr));
            }
        }

        // --- kolizje ---
        for (auto& a : entities)
            for (auto& b : entities)
            {
                // pocisk vs asteroida
                if (a->name == "asteroid" && b->name == "bullet")
                    if (isCollide(a.get(), b.get()))
                    {
                        a->life = false;
                        b->life = false;
                        destroyedAsteroids++;

                        auto explosionPtr = make_unique<Entity>();
                        explosionPtr->settings(sExplosion, a->x, a->y);
                        explosionPtr->name = "explosion";
                        entities.push_back(std::move(explosionPtr));

                        // rozpad asteroidy na mniejsze
                        for (int i = 0; i < 2; i++) {
                            if (a->R == 15) continue;
                            auto smallAsteroidPtr = make_unique<asteroid>();
                            smallAsteroidPtr->settings(sRock_small, a->x, a->y, rand() % 360, 15);
                            entities.push_back(std::move(smallAsteroidPtr));
                        }
                    }

                // gracz vs asteroida
                if (a->name == "player" && b->name == "asteroid")
                    if (isCollide(a.get(), b.get()))
                    {
                        b->life = false;

                        auto explosionPtr = make_unique<Entity>();
                        explosionPtr->settings(sExplosion_ship, a->x, a->y);
                        explosionPtr->name = "explosion";
                        entities.push_back(std::move(explosionPtr));

                        app.close();
                    }
            }

        // --- animacja statku ---
        if (p->thrust) p->anim = sPlayer_go;
        else p->anim = sPlayer;

        // --- update i usuwanie obiektów (poprawione z oryginału) ---
        for (auto it = entities.begin(); it != entities.end(); )
        {
            (*it)->update();
            (*it)->anim.update();

            // jeśli to eksplozja i animacja się skończyła → usuń
            if ((*it)->name == "explosion" && (*it)->anim.isEnd())
                (*it)->life = false;

            if (!(*it)->life) it = entities.erase(it);
            else ++it;
        }

        // --- rysowanie ---
        app.clear();
        for (auto& i : entities) i->draw(app);
        app.display();
    }

    return 0;
}
