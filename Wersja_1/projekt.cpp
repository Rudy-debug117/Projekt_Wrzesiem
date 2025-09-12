#include <SFML/Graphics.hpp>
#include <list>
#include <ctime>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <memory>
#include <iostream>
#include <cmath>

#include "Player.h"
#include "Animation.h"
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

    sf::Texture t1, t2, t3, t4, t5, t6, t7;
    if (!t1.loadFromFile("D:/LuL/Downloads/Asteroids2.2/Asteroids2.2/images/spaceship.png")) {
        cerr << "Blad ladowania: spaceship.png" << endl;
        return 1;
    }


    sf::Sprite background(t2);



    Animation sRock(t4, 0, 0, 64, 64, 16, 0.2);
    Animation sRock_small(t6, 0, 0, 64, 64, 16, 0.2);
    Animation sPlayer(t1, 40, 0, 40, 40, 1, 0);
    Animation sPlayer_go(t1, 40, 40, 40, 40, 1, 0);
    Animation sExplosion_ship(t7, 0, 0, 192, 192, 64, 0.5);

    list<unique_ptr<Entity>> entities;


    auto playerPtr = make_unique<player>();
    playerPtr->settings(sPlayer, W / 2, H / 2, 0, 20);
    player* p = playerPtr.get();
    p->speed = playerSpeed;
    entities.push_back(std::move(playerPtr));

    float safeTime = 5.0f;
    sf::Clock clock;






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








        if (p->thrust) p->anim = sPlayer_go;
        else p->anim = sPlayer;


        for (auto& e : entities)
            if (e->name == "explosion")
                if (e->anim.isEnd()) e->life = 0;



        for (auto it = entities.begin(); it != entities.end(); )
        {
            (*it)->update();
            (*it)->anim.update();

            if (!(*it)->life) it = entities.erase(it);
            else ++it;
        }

        app.clear();
        app.draw(background);
        for (auto& i : entities) i->draw(app);





        app.display();
    }

    return 0;
}

