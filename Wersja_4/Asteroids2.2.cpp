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

// --- funkcje do obsługi wyników ---
int readHighscore(const string& filename) {
    ifstream file(filename);
    int highscore = 0;
    if (file.is_open()) {
        string line;
        if (getline(file, line)) {
            if (line.find("Highscore:") == 0) {
                try { highscore = stoi(line.substr(10)); }
                catch (...) {}
            }
        }
    }
    return highscore;
}

void saveHighscore(const string& filename, int highscore) {
    ofstream file(filename);
    if (file.is_open()) {
        file << "Highscore:" << highscore << endl;
    }
}

int main()
{
    srand(time(0));

    float playerFireRate = 0.4f;
    float playerSpeed = 2.0f;
    const float maxPlayerSpeed = 6.0f;
    float fireCooldown = 0.0f;

    sf::RenderWindow app(sf::VideoMode(W, H), "Asteroids!");
    app.setFramerateLimit(60);

    sf::Texture t1, t2, t3, t4, t5, t6;
    if (!t1.loadFromFile("C:/projekcik/images/spaceship.png")) return 1;
    if (!t2.loadFromFile("C:/projekcik/images/explosions/type_C.png")) return 1;
    if (!t3.loadFromFile("C:/projekcik/images/rock.png")) return 1;
    if (!t4.loadFromFile("C:/projekcik/images/rock_small.png")) return 1;
    if (!t5.loadFromFile("C:/projekcik/images/explosions/type_B.png")) return 1;
    if (!t6.loadFromFile("C:/projekcik/images/fire_red.png")) return 1;

    Animation sRock(t3, 0, 0, 64, 64, 16, 0.2);
    Animation sRock_small(t4, 0, 0, 64, 64, 16, 0.2);
    Animation sPlayer(t1, 40, 0, 40, 40, 1, 0);
    Animation sPlayer_go(t1, 40, 40, 40, 40, 1, 0);
    Animation sExplosion_ship(t5, 0, 0, 192, 192, 64, 0.5);
    Animation sExplosion(t2, 0, 0, 256, 256, 48, 0.5);
    Animation sBullet(t6, 0, 0, 32, 64, 16, 0.8);

    sf::Font orbitronFont;
    if (!orbitronFont.loadFromFile("C:/projekcik/Orbitron/static/Orbitron-Regular.ttf")) return 1;

    list<unique_ptr<Entity>> entities;

    auto playerPtr = make_unique<player>();
    playerPtr->settings(sPlayer, W / 2, H / 2, 0, 20);
    player* p = playerPtr.get();
    p->speed = playerSpeed;
    entities.push_back(std::move(playerPtr));

    float safeTime = 5.0f;
    sf::Clock clock;

    int destroyedAsteroids = 0;
    const string scoreFile = "scores.txt";
    int highscore = readHighscore(scoreFile);

    while (app.isOpen())
    {
        sf::Event event;
        while (app.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                app.close();

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

        if (fireCooldown > 0.0f)
            fireCooldown -= 1.0f / 60.0f;

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

        // --- kolizje ---
        for (auto& a : entities)
            for (auto& b : entities)
            {
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

                        for (int i = 0; i < 2; i++) {
                            if (a->R == 15) continue;
                            auto smallAsteroidPtr = make_unique<asteroid>();
                            smallAsteroidPtr->settings(sRock_small, a->x, a->y, rand() % 360, 15);
                            entities.push_back(std::move(smallAsteroidPtr));
                        }
                    }

                if (a->name == "player" && b->name == "asteroid")
                    if (isCollide(a.get(), b.get()))
                    {
                        b->life = false;
                        auto explosionPtr = make_unique<Entity>();
                        explosionPtr->settings(sExplosion_ship, a->x, a->y);
                        explosionPtr->name = "explosion";
                        entities.push_back(std::move(explosionPtr));

                        // --- obliczanie końcowego wyniku ---
                        int timeScore = static_cast<int>(clock.getElapsedTime().asSeconds()) * 100;
                        int asteroidScore = destroyedAsteroids * 50;
                        int playerScore = timeScore + asteroidScore;

                        if (playerScore > highscore) highscore = playerScore;
                        saveHighscore(scoreFile, highscore);

                        app.close();
                    }
            }

        if (p->thrust) p->anim = sPlayer_go;
        else p->anim = sPlayer;

        for (auto it = entities.begin(); it != entities.end(); )
        {
            (*it)->update();
            (*it)->anim.update();

            if ((*it)->name == "explosion" && (*it)->anim.isEnd())
                (*it)->life = false;

            if (!(*it)->life) it = entities.erase(it);
            else ++it;
        }

        // --- liczenie punktów ---
        int timeScore = static_cast<int>(clock.getElapsedTime().asSeconds()) * 100;
        int asteroidScore = destroyedAsteroids * 50;
        int currentScore = timeScore + asteroidScore;

        // --- timer do wyświetlania ---
        int minutes = (timeScore / 100) / 60;
        int seconds = (timeScore / 100) % 60;

        // --- rysowanie ---
        app.clear();
        for (auto& i : entities) i->draw(app);

        sf::Text scoreText;
        scoreText.setFont(orbitronFont);
        scoreText.setCharacterSize(24);
        scoreText.setFillColor(sf::Color::White);
        scoreText.setPosition(10, 10);
        scoreText.setString("Score: " + std::to_string(currentScore));
        app.draw(scoreText);

        sf::Text destroyedText;
        destroyedText.setFont(orbitronFont);
        destroyedText.setCharacterSize(24);
        destroyedText.setFillColor(sf::Color::White);
        destroyedText.setPosition(10, 40);
        destroyedText.setString("Asteroids destroyed: " + std::to_string(destroyedAsteroids));
        app.draw(destroyedText);

        sf::Text highscoreText;
        highscoreText.setFont(orbitronFont);
        highscoreText.setCharacterSize(24);
        highscoreText.setFillColor(sf::Color::Yellow);
        highscoreText.setPosition(10, 70);
        highscoreText.setString("Highscore: " + std::to_string(highscore));
        app.draw(highscoreText);

        sf::Text timerText;
        timerText.setFont(orbitronFont);
        timerText.setCharacterSize(24);
        timerText.setFillColor(sf::Color::Cyan);
        timerText.setPosition(10, 100);
        timerText.setString("Time: " + to_string(minutes) + ":" + (seconds < 10 ? "0" : "") + to_string(seconds));
        app.draw(timerText);

        app.display();
    }

    return 0;
}
