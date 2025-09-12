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

#include "Player.h"
#include "Animation.h"
#include "Collision.h"
#include "Header.h"

#include <iostream>

using namespace std;

int main()
{
    srand(time(0));


    float playerFireRate = 0.4f;
    float playerSpeed = 2.0f;
    const float maxPlayerSpeed = 6.0f;
    float fireCooldown = 0.0f;

    sf::RenderWindow app(sf::VideoMode(W, H), "Asteroids!");
    app.setFramerateLimit(60);

    sf::Texture t1, t2, t3, t4, t5;
    if (!t1.loadFromFile("../txt/spaceship.png")) {
        cerr << "Blad ladowania: spaceship.png" << endl;
        return 1;
    }
    if (!t2.loadFromFile("../txt/type_C.png")) {
        cerr << "Blad ladowania: explosions/type_C.png" << endl;
        return 1;
    }
    if (!t3.loadFromFile("../txt/rock.png")) {
        cerr << "Blad ladowania: rock.png" << endl;
        return 1;
    }

    if (!t4.loadFromFile("../txt/rock_small.png")) {
        cerr << "Blad ladowania: rock_small.png" << endl;
        return 1;
    }
    if (!t5.loadFromFile("../txt/type_B.png")) {
        cerr << "Blad ladowania: tybe_B.png" << endl;
        return 1;
    }
    



    Animation sRock(t3, 0, 0, 64, 64, 16, 0.2);
    Animation sRock_small(t4, 0, 0, 64, 64, 16, 0.2);
    Animation sPlayer(t1, 40, 0, 40, 40, 1, 0);
    Animation sPlayer_go(t1, 40, 40, 40, 40, 1, 0);
    Animation sExplosion_ship(t5, 0, 0, 192, 192, 64, 0.5);

    list<unique_ptr<Entity>> entities;


    auto playerPtr = make_unique<player>();
    playerPtr->settings(sPlayer, W / 2, H / 2, 0, 20);
    player* p = playerPtr.get();
    p->speed = playerSpeed;
    entities.push_back(std::move(playerPtr));

    float safeTime = 5.0f;
    sf::Clock clock;


    sf::Font orbitronFont;
    if (!orbitronFont.loadFromFile("../txt/Orbitron-Regular.ttf")) {
        cerr << "Blad ladowania czcionki: Orbitron-Regular.ttf" << endl;
        return 1;
    }

    int destroyedAsteroids = 0;

    while (app.isOpen())
    {
        sf::Event event;
        while (app.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                app.close();

        }




        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) p->angle += 3;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) p->angle -= 3;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) p->thrust = true;
        else p->thrust = false;


        if (clock.getElapsedTime().asSeconds() > safeTime)
        {
            if (rand() % 150 == 0)
            {
                auto asteroidPtr = make_unique<asteroid>();
                asteroidPtr->settings(sRock, 0, rand() % H, rand() % 360, 25);
                entities.push_back(std::move(asteroidPtr));
            }
        }


        for (auto& a : entities)
            for (auto& b : entities)
            {

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


        if (p->thrust) p->anim = sPlayer_go;
        else p->anim = sPlayer;




        for (auto it = entities.begin(); it != entities.end(); )
        {
            (*it)->update();
            (*it)->anim.update();

            if (!(*it)->life) it = entities.erase(it);
            else ++it;
        }

        app.clear();
        
        for (auto& i : entities) i->draw(app);





        app.display();
    }

    return 0;
}

