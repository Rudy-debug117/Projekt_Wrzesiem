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
#include "Constants.h"

using namespace std;

// --------------------
// Funkcja: odczyt wyników z pliku
// Zwraca: vector z 10 najlepszymi wynikami, ustawia highscore, lastScore, lastDestroyed
// --------------------
vector<int> readScores(const string& filename, int& highscore, int& lastScore, int& lastDestroyed)
{
    vector<int> scores;
    ifstream file(filename);

    highscore = 0;
    lastScore = 0;
    lastDestroyed = 0;

    if (!file.is_open())
    {
        cerr << "Nie można otworzyć pliku z wynikami: " << filename << endl;
        return scores;
    }

    string line;

    // Odczyt highscore
    if (!getline(file, line)) return scores;
    if (line.find("Highscore:") == 0)
    {
        try { highscore = stoi(line.substr(10)); }
        catch (...) { highscore = 0; }
    }

    // Odczyt ostatniego wyniku
    if (!getline(file, line)) return scores;
    if (line.find("LastScore:") == 0)
    {
        try { lastScore = stoi(line.substr(10)); }
        catch (...) { lastScore = 0; }
    }

    // Odczyt liczby zniszczonych asteroid w ostatniej grze
    if (!getline(file, line)) return scores;
    if (line.find("LastDestroyed:") == 0)
    {
        try { lastDestroyed = stoi(line.substr(13)); }
        catch (...) { lastDestroyed = 0; }
    }

    // Odczyt 10 najlepszych wyników
    while (getline(file, line) && scores.size() < 10)
    {
        try {
            scores.push_back(stoi(line));
        }
        catch (...) {}
    }

    return scores;
}

// --------------------
// Funkcja: zapis wyników do pliku
// --------------------
void saveScores(const string& filename, int highscore, const vector<int>& scores, int lastScore, int lastDestroyed)
{
    ofstream file(filename);

    if (!file.is_open())
    {
        cerr << "Nie można zapisać wyników do pliku: " << filename << endl;
        return;
    }

    file << "Highscore: " << highscore << "\n";
    file << "LastScore: " << lastScore << "\n";
    file << "LastDestroyed: " << lastDestroyed << "\n";
    for (int score : scores)
    {
        file << score << "\n";
    }
}

// --------------------
// Funkcja: aktualizacja listy wyników i highscore
// --------------------
void updateScores(vector<int>& scores, int newScore, int& highscore)
{
    scores.push_back(newScore);
    sort(scores.rbegin(), scores.rend());

    if (scores.size() > 10)
        scores.resize(10);

    if (newScore > highscore)
        highscore = newScore;
}

// --------------------
// Główna funkcja gry
// --------------------
int main()
{
    srand(time(0));

    // --- Ulepszenia: podstawowe wartości ---
    float playerFireRate = 0.4f; // czas między strzałami (im mniej, tym szybciej)
    float playerSpeed = 2.0f;    // bazowa prędkość statku
    const float maxPlayerSpeed = 6.0f; // maksymalna prędkość statku
    float fireCooldown = 0.0f;   // licznik cooldownu na strzał

    int upgradePoints = 0;       // Punkty do ulepszeń (zdobywane za wynik)
    bool doubleShot = false;     // Czy aktywne ulepszenie Double Shot

    // Okno gry
    sf::RenderWindow app(sf::VideoMode(W, H), "Asteroids!");
    app.setFramerateLimit(60);

    // Zmienne do wyników
    const string scoreFile = "scores.txt";
    int highscore, lastScore, lastDestroyed;
    vector<int> scores = readScores(scoreFile, highscore, lastScore, lastDestroyed);

    // --- Ładowanie tekstur ---
    sf::Texture t1, t2, t3, t4, t5, t6, t7;
    if (!t1.loadFromFile("C:/projekcik/images/spaceship.png")) {
        cerr << "Błąd ładowania: spaceship.png" << endl;
        return 1;
    }
    if (!t2.loadFromFile("C:/projekcik/images/background.jpg")) {
        cerr << "Błąd ładowania: background.jpg" << endl;
        return 1;
    }
    if (!t3.loadFromFile("C:/projekcik/images/explosions/type_C.png")) {
        cerr << "Błąd ładowania: explosions/type_C.png" << endl;
        return 1;
    }
    if (!t4.loadFromFile("C:/projekcik/images/rock.png")) {
        cerr << "Błąd ładowania: rock.png" << endl;
        return 1;
    }
    if (!t5.loadFromFile("C:/projekcik/images/fire_red.png")) {
        cerr << "Błąd ładowania: fire_red.png" << endl;
        return 1;
    }
    if (!t6.loadFromFile("C:/projekcik/images/rock_small.png")) {
        cerr << "Błąd ładowania: rock_small.png" << endl;
        return 1;
    }
    if (!t7.loadFromFile("C:/projekcik/images/explosions/type_B.png")) {
        cerr << "Błąd ładowania: explosions/type_B.png" << endl;
        return 1;
    }

    sf::Sprite background(t2);

    // --- Tworzenie animacji ---
    Animation sExplosion(t3, 0, 0, 256, 256, 48, 0.5);
    Animation sRock(t4, 0, 0, 64, 64, 16, 0.2);
    Animation sRock_small(t6, 0, 0, 64, 64, 16, 0.2);
    Animation sBullet(t5, 0, 0, 32, 64, 16, 0.8);
    Animation sPlayer(t1, 40, 0, 40, 40, 1, 0);
    Animation sPlayer_go(t1, 40, 40, 40, 40, 1, 0);
    Animation sExplosion_ship(t7, 0, 0, 192, 192, 64, 0.5);

    list<unique_ptr<Entity>> entities; // Lista wszystkich obiektów gry

    // --- Tworzenie gracza ---
    auto playerPtr = make_unique<player>();
    playerPtr->settings(sPlayer, W / 2, H / 2, 0, 20);
    player* p = playerPtr.get();
    p->speed = playerSpeed; // przekazanie prędkości do gracza
    entities.push_back(std::move(playerPtr));

    float safeTime = 5.0f; // czas bez asteroid na starcie
    sf::Clock clock;       // licznik czasu gry

    // --- Ładowanie czcionki ---
    sf::Font orbitronFont;
    if (!orbitronFont.loadFromFile("C:/projekcik/Orbitron/static/Orbitron-Regular.ttf")) {
        cerr << "Błąd ładowania czcionki: Orbitron-Regular.ttf" << endl;
        return 1;
    }

    int destroyedAsteroids = 0; // liczba zniszczonych asteroid w bieżącej grze

    // --- Zmienne stanu gry ---
    bool isMenu = true;
    bool isGameRunning = false;
    bool isViewingScores = false;
    bool isUpgrades = false;
    int selectedOption = 0;

    // --- Teksty menu ---
    sf::Text menuStart("Start Game", orbitronFont, 30);
    sf::Text menuUpgrades("Upgrades", orbitronFont, 30);
    sf::Text menuStats("View Scores", orbitronFont, 30);
    sf::Text menuExit("Exit", orbitronFont, 30);
    menuStart.setPosition(W / 2 - 75, H / 2.5);
    menuUpgrades.setPosition(W / 2 - 75, H / 2.1);
    menuStats.setPosition(W / 2 - 75, H / 1.8);
    menuExit.setPosition(W / 2 - 75, H / 1.5);
    menuStart.setFillColor(sf::Color::White);
    menuUpgrades.setFillColor(sf::Color::White);
    menuStats.setFillColor(sf::Color::White);
    menuExit.setFillColor(sf::Color::White);

    sf::Color activeColor = sf::Color::Yellow;
    sf::Color inactiveColor = sf::Color::White;

    // --------------------
    // Pętla główna gry
    // --------------------
    while (app.isOpen())
    {
        sf::Event event;
        while (app.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                app.close();

            // --- Obsługa klawiatury ---
            if (event.type == sf::Event::KeyPressed)
            {
                // --- Menu główne ---
                if (isMenu) {
                    if (event.key.code == sf::Keyboard::Up) {
                        selectedOption = (selectedOption - 1 + 4) % 4;
                    }
                    if (event.key.code == sf::Keyboard::Down) {
                        selectedOption = (selectedOption + 1) % 4;
                    }
                    if (event.key.code == sf::Keyboard::Enter) {
                        if (selectedOption == 0) {
                            // Start gry od nowa
                            isMenu = false;
                            isGameRunning = true;

                            // Wyczyść wszystkie obiekty
                            entities.clear();

                            // Stwórz nowego gracza
                            auto playerPtrNew = make_unique<player>();
                            playerPtrNew->settings(sPlayer, W / 2, H / 2, 0, 20);
                            player* newP = playerPtrNew.get();
                            newP->speed = playerSpeed;
                            entities.push_back(std::move(playerPtrNew));
                            p = newP; // zaktualizuj wskaźnik na gracza

                            destroyedAsteroids = 0;
                            clock.restart();
                            fireCooldown = 0.0f;
                        }
                        else if (selectedOption == 1) {
                            // Przejście do ulepszeń
                            isMenu = false;
                            isUpgrades = true;
                        }
                        else if (selectedOption == 2) {
                            // Przeglądanie wyników
                            isMenu = false;
                            isViewingScores = true;
                        }
                        else if (selectedOption == 3) {
                            // Wyjście z gry
                            app.close();
                        }
                    }
                }

                // --- Strzelanie i wyjście z gry podczas rozgrywki ---
                if (isGameRunning) {
                    if (event.key.code == sf::Keyboard::Space && fireCooldown <= 0.0f) {
                        // Strzał: pojedynczy lub podwójny (double shot)
                        if (doubleShot) {
                            float offset = 15.0f;
                            float rad = p->angle * 3.14159265f / 180.0f;
                            float dx = std::cos(rad + 3.14159265f / 2) * offset;
                            float dy = std::sin(rad + 3.14159265f / 2) * offset;

                            auto bulletPtr1 = make_unique<bullet>();
                            bulletPtr1->settings(sBullet, p->x + dx, p->y + dy, p->angle, 10);
                            entities.push_back(std::move(bulletPtr1));

                            auto bulletPtr2 = make_unique<bullet>();
                            bulletPtr2->settings(sBullet, p->x - dx, p->y - dy, p->angle, 10);
                            entities.push_back(std::move(bulletPtr2));
                        }
                        else {
                            auto bulletPtr = make_unique<bullet>();
                            bulletPtr->settings(sBullet, p->x, p->y, p->angle, 10);
                            entities.push_back(std::move(bulletPtr));
                        }
                        fireCooldown = playerFireRate;
                    }
                    if (event.key.code == sf::Keyboard::Escape) {
                        isGameRunning = false;
                        isMenu = true;
                    }
                }
                // --- Wyjście z ekranu wyników ---
                if (isViewingScores) {
                    if (event.key.code == sf::Keyboard::Escape) {
                        isViewingScores = false;
                        isMenu = true;
                    }
                }
            }
        }

        // --- Rysowanie i obsługa menu głównego ---
        if (isMenu) {
            app.clear();
            app.draw(background);

            sf::Text logoText("ASTEROIDS", orbitronFont, 100);
            logoText.setFillColor(sf::Color::White);
            logoText.setStyle(sf::Text::Bold);
            logoText.setPosition(W / 2 - logoText.getLocalBounds().width / 2, H / 6);
            app.draw(logoText);

            menuStart.setFillColor(selectedOption == 0 ? activeColor : inactiveColor);
            menuUpgrades.setFillColor(selectedOption == 1 ? activeColor : inactiveColor);
            menuStats.setFillColor(selectedOption == 2 ? activeColor : inactiveColor);
            menuExit.setFillColor(selectedOption == 3 ? activeColor : inactiveColor);

            app.draw(menuStart);
            app.draw(menuUpgrades);
            app.draw(menuStats);
            app.draw(menuExit);
        }

        // --- Okno ulepszeń (Upgrades) ---
        if (isUpgrades) {
            app.clear();
            app.draw(background);

            sf::Text upgradesTitle("UPGRADES", orbitronFont, 40);
            upgradesTitle.setFillColor(sf::Color::Yellow);
            upgradesTitle.setPosition(W / 2 - 100, H / 6);
            app.draw(upgradesTitle);

            sf::Text pointsText("Upgrade Points: " + std::to_string(upgradePoints), orbitronFont, 28);
            pointsText.setFillColor(sf::Color::White);
            pointsText.setPosition(W / 2 - 120, H / 3.5);
            app.draw(pointsText);

            sf::Text fireRateText("Fire Rate (Q): " + std::to_string(playerFireRate) + "s", orbitronFont, 24);
            fireRateText.setFillColor(sf::Color::White);
            fireRateText.setPosition(W / 2 - 120, H / 2.5);
            app.draw(fireRateText);

            sf::Text speedText("Speed (E): " + std::to_string(playerSpeed), orbitronFont, 24);
            speedText.setFillColor(sf::Color::White);
            speedText.setPosition(W / 2 - 120, H / 2.1);
            app.draw(speedText);

            sf::Text doubleShotText("Double Shot (R): " + std::string(doubleShot ? "ON" : "OFF") + " (koszt: 5)", orbitronFont, 24);
            doubleShotText.setFillColor(sf::Color::White);
            doubleShotText.setPosition(W / 2 - 120, H / 1.8);
            app.draw(doubleShotText);

            sf::Text infoText("Q - Fire Rate | E - Speed | R - Double Shot | ESC - Powrót", orbitronFont, 20);
            infoText.setFillColor(sf::Color::White);
            infoText.setPosition(W / 2 - 220, H - 80);
            app.draw(infoText);

            app.display();

            // --- Obsługa przycisków ulepszeń ---
            static bool qPressed = false, ePressed = false, rPressed = false;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) {
                if (!qPressed && upgradePoints > 0 && playerFireRate > 0.11f) {
                    playerFireRate -= 0.05f;
                    if (playerFireRate < 0.1f) playerFireRate = 0.1f;
                    upgradePoints--;
                }
                qPressed = true;
            }
            else {
                qPressed = false;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::E)) {
                if (!ePressed && upgradePoints > 0 && playerSpeed < maxPlayerSpeed) {
                    playerSpeed += 0.5f;
                    if (playerSpeed > maxPlayerSpeed) playerSpeed = maxPlayerSpeed;
                    upgradePoints--;
                }
                ePressed = true;
            }
            else {
                ePressed = false;
            }
            //*********//
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
                if (!rPressed && upgradePoints >= 5 && !doubleShot) {
                    doubleShot = true;
                    upgradePoints -= 5;
                }
                rPressed = true;
            }
            //*********//
            else {
                rPressed = false;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
                isUpgrades = false;
                isMenu = true;
                sf::sleep(sf::milliseconds(200));
            }
            continue;
        }

        // --- Okno wyników ---
        if (isViewingScores) {
            app.clear();
            app.draw(background);
            sf::Text highscoreText("Highscore: " + to_string(highscore), orbitronFont, 30);
            highscoreText.setFillColor(sf::Color::White);
            highscoreText.setPosition(W / 2 - 75, H / 4);
            app.draw(highscoreText);

            sf::Text lastScoreText("Previous score: " + to_string(lastScore), orbitronFont, 24);
            lastScoreText.setFillColor(sf::Color::White);
            lastScoreText.setPosition(W / 2 - 75, H / 3.2f);
            app.draw(lastScoreText);

            sf::Text lastDestroyedText("Last destroyed: " + to_string(lastDestroyed), orbitronFont, 24);
            lastDestroyedText.setFillColor(sf::Color::White);
            lastDestroyedText.setPosition(W / 2 - 75, H / 2.8f);
            app.draw(lastDestroyedText);

            sf::Text scoresText;
            scoresText.setFont(orbitronFont);
            scoresText.setCharacterSize(24);
            scoresText.setFillColor(sf::Color::White);
            std::stringstream scoresStream;
            for (size_t i = 0; i < scores.size(); ++i)
            {
                scoresStream << (i + 1) << ". " << scores[i] << "\n";
            }
            scoresText.setString(scoresStream.str());
            scoresText.setPosition(W / 2 - 75, H / 2.3f);
            app.draw(scoresText);

            sf::Text backText("Press Escape to Return", orbitronFont, 20);
            backText.setFillColor(sf::Color::White);
            backText.setPosition(W / 2 - 75, H - 40);
            app.draw(backText);
        }

        // --- Główna rozgrywka ---
        if (isGameRunning) {
            // Odliczanie cooldownu na strzał
            if (fireCooldown > 0.0f)
                fireCooldown -= 1.0f / 60.0f;

            // Sterowanie statkiem
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) p->angle += 3;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) p->angle -= 3;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) p->thrust = true;
            else p->thrust = false;

            // Losowe generowanie asteroid po czasie ochronnym
            if (clock.getElapsedTime().asSeconds() > safeTime)
            {
                if (rand() % 150 == 0)
                {
                    auto asteroidPtr = make_unique<asteroid>();
                    asteroidPtr->settings(sRock, 0, rand() % H, rand() % 360, 25);
                    entities.push_back(std::move(asteroidPtr));
                }
            }

            // --- Kolizje i logika gry ---
            for (auto& a : entities)
                for (auto& b : entities)
                {
                    // Kolizja: pocisk vs asteroida
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

                            // Rozpad dużej asteroidy na dwie małe
                            for (int i = 0; i < 2; i++)
                            {
                                if (a->R == 15) continue;
                                auto smallAsteroidPtr = make_unique<asteroid>();
                                smallAsteroidPtr->settings(sRock_small, a->x, a->y, rand() % 360, 15);
                                entities.push_back(std::move(smallAsteroidPtr));
                            }
                        }

                    // Kolizja: gracz vs asteroida
                    if (a->name == "player" && b->name == "asteroid")
                        if (isCollide(a.get(), b.get()))
                        {
                            b->life = false;

                            auto explosionPtr = make_unique<Entity>();
                            explosionPtr->settings(sExplosion_ship, a->x, a->y);
                            explosionPtr->name = "explosion";
                            entities.push_back(std::move(explosionPtr));

                            // Obliczanie wyniku
                            int timeScore = static_cast<int>(clock.getElapsedTime().asSeconds()) * 100;
                            int asteroidScore = destroyedAsteroids * 50;
                            int playerScore = timeScore + asteroidScore;
                            updateScores(scores, playerScore, highscore);
                            saveScores(scoreFile, highscore, scores, playerScore, destroyedAsteroids);

                            lastScore = playerScore;
                            lastDestroyed = destroyedAsteroids;

                            // Punkty do ulepszeń: 1 za każde 2000 punktów
                            upgradePoints += playerScore / 2000;

                            // Powrót do menu po kolizji z asteroidą
                            isGameRunning = false;
                            isMenu = true;
                        }
                }

            // Animacja statku (ruch/bez ruchu)
            if (p->thrust) p->anim = sPlayer_go;
            else p->anim = sPlayer;

            // Usuwanie zakończonych eksplozji
            for (auto& e : entities)
                if (e->name == "explosion")
                    if (e->anim.isEnd()) e->life = 0;

            // Obliczanie aktualnego wyniku
            int timeScore = static_cast<int>(clock.getElapsedTime().asSeconds()) * 100;
            int asteroidScore = destroyedAsteroids * 50;
            int currentScore = timeScore + asteroidScore;

            int minutes = (timeScore / 100) / 60;
            int seconds = (timeScore / 100) % 60;

            // --- Wyświetlanie HUD ---
            sf::Text highscoreText;
            highscoreText.setFont(orbitronFont);
            highscoreText.setCharacterSize(24);
            highscoreText.setFillColor(sf::Color::White);
            highscoreText.setPosition(10, 40);
            stringstream highscoreStream;
            highscoreStream << "Highscore: " << highscore;
            highscoreText.setString(highscoreStream.str());

            sf::Text timeText;
            timeText.setFont(orbitronFont);
            timeText.setCharacterSize(24);
            timeText.setFillColor(sf::Color::White);
            timeText.setPosition(10, 70);
            stringstream timeStream;
            timeStream << "Time: " << minutes << ":" << (seconds < 10 ? "0" : "") << seconds;
            timeText.setString(timeStream.str());

            sf::Text scoreText;
            scoreText.setFont(orbitronFont);
            scoreText.setCharacterSize(24);
            scoreText.setFillColor(sf::Color::White);
            scoreText.setPosition(10, 100);
            scoreText.setString("Score: " + std::to_string(currentScore));

            sf::Text destroyedText;
            destroyedText.setFont(orbitronFont);
            destroyedText.setCharacterSize(24);
            destroyedText.setFillColor(sf::Color::White);
            destroyedText.setPosition(10, 130);
            destroyedText.setString("Asteroids destroyed: " + std::to_string(destroyedAsteroids));

            sf::Text quitHint("Press ESC to quit", orbitronFont, 20);
            quitHint.setFillColor(sf::Color::White);
            quitHint.setPosition(W - 200, H - 30);

            // --- Aktualizacja i rysowanie obiektów gry ---
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

            app.draw(highscoreText);
            app.draw(timeText);
            app.draw(scoreText);
            app.draw(destroyedText);
            app.draw(quitHint);
        }

        app.display();
    }

    return 0;
}