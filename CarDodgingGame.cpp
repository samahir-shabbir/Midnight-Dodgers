// ============================================================
//  main.cpp  –  Car Dodge Game  (SFML 2.x compatible)
//  Screens: Main Menu | Enter Name | Playing | Paused |
//           Game Over | Options | Help | High Score
// ============================================================

#include <SFML/Graphics.hpp>
#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <cmath>

using namespace std;

// ─── GAME STATES ─────────────────────────────────────────────
enum GameState
{
    MAIN_MENU,
    ENTER_NAME,
    PLAYING,
    PAUSED,
    GAME_OVER,
    OPTIONS,
    HELP,
    HIGH_SCORE_SCREEN
};

// ─── HIGH SCORE ENTRY ────────────────────────────────────────
struct ScoreEntry
{
    string name;
    int    score;
};

// ─── HELPERS ─────────────────────────────────────────────────

// Centre a text object on a given X position
void centreTextX(sf::Text& t, float cx)
{
    sf::FloatRect b = t.getLocalBounds();
    t.setOrigin(b.left + b.width / 2.f, 0.f);
    t.setPosition(cx, t.getPosition().y);
}

// Reset all enemies to their start positions
void resetEnemies(std::vector<sf::Sprite>& enemies)
{
    float startY[7] = { 0.f, -200.f, -400.f, -600.f, -800.f, -1000.f, -2000.f };
    float startX[7] = { 100.f, 445.f, 750.f, 1050.f, 1350.f, 1610.f, 750.f };
    for (int i = 0; i < (int)enemies.size(); i++)
        enemies[i].setPosition(startX[i], startY[i]);
}

// ─── MAIN ────────────────────────────────────────────────────
int main()
{
    cout << filesystem::current_path() << endl;

    // ── Window ───────────────────────────────────────────────
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Car Dodge Game");
    window.setFramerateLimit(60);

    GameState gameState = MAIN_MENU;

    // ── Player data ──────────────────────────────────────────
    string playerName = "";
    int    score = 0;
    int    lives = 3;
    bool   gameOver = false;

    // ── High score list (top 5) ───────────────────────────────
    std::vector<ScoreEntry> highScores;

    // ── Textures & Sprites ───────────────────────────────────
    sf::Texture carTexture;
    carTexture.loadFromFile("ASA_Car.png");
    sf::Sprite car(carTexture);
    car.setPosition(1050.f, 615.f);
    car.setScale(0.26f, 0.26f);

    sf::Texture roadTexture;
    roadTexture.loadFromFile("road.png");
    roadTexture.setSmooth(false);
    sf::Sprite road(roadTexture), road2(roadTexture);
    road.setPosition(0.f, 0.f);
    road2.setPosition(0.f, -1065.f);

    sf::Texture sparkTexture;
    sparkTexture.loadFromFile("Boom.png");
    sf::Sprite spark(sparkTexture);
    spark.setScale(0.15f, 0.15f);
    spark.setOrigin(spark.getLocalBounds().width / 2.f,
        spark.getLocalBounds().height / 2.f);
    bool      showSpark = false;
    sf::Clock sparkClock;

    // Enemy textures
    sf::Texture texBlack, texBlue, texGreen, texYellow, texPurple, texOrange, texWhite;
    texBlack.loadFromFile("Black.png");
    texBlue.loadFromFile("Blue.png");
    texGreen.loadFromFile("Green.png");
    texYellow.loadFromFile("Yellow.png");
    texPurple.loadFromFile("Purple.png");
    texOrange.loadFromFile("Orange.png");
    texWhite.loadFromFile("White.png");

    // Enemy sprites
    std::vector<sf::Sprite> enemies;
    {
        sf::Sprite e0(texBlack);  e0.setScale(0.44f, 0.44f); enemies.push_back(e0);
        sf::Sprite e1(texBlue);   e1.setScale(0.44f, 0.44f); enemies.push_back(e1);
        sf::Sprite e2(texGreen);  e2.setScale(0.44f, 0.44f); enemies.push_back(e2);
        sf::Sprite e3(texPurple); e3.setScale(0.44f, 0.44f); enemies.push_back(e3);
        sf::Sprite e4(texYellow); e4.setScale(0.33f, 0.33f); enemies.push_back(e4);
        sf::Sprite e5(texOrange); e5.setScale(0.44f, 0.44f); enemies.push_back(e5);
        sf::Sprite e6(texWhite);  e6.setScale(0.31f, 0.31f); enemies.push_back(e6);
    }
    resetEnemies(enemies);

    int   lanes[6] = { 100, 445, 750, 1050, 1350, 1610 };
    float playerSpeed = 700.f;
    float enemySpeed = 700.f;

    // ── Safe mode ────────────────────────────────────────────
    bool      safeMode = false;
    sf::Clock safeClock;

    // ── Font ─────────────────────────────────────────────────
    sf::Font font;
    if (!font.loadFromFile("PressStart2P-Regular.ttf"))
        cerr << "Font not found!\n";

    // ── Clock ────────────────────────────────────────────────
    sf::Clock clock;

    // ── Dark overlay ─────────────────────────────────────────
    sf::RectangleShape overlay(sf::Vector2f(1920.f, 1080.f));
    overlay.setFillColor(sf::Color(0, 0, 0, 190));

    // ── Status bar (shown at bottom during PLAYING & PAUSED) ─
    sf::RectangleShape statusBar(sf::Vector2f(1920.f, 70.f));
    statusBar.setFillColor(sf::Color(0, 0, 0, 200));
    statusBar.setPosition(0.f, 1010.f);

    // ─────────────────────────────────────────────────────────
    //  TEXT & BUTTON HELPERS
    // ─────────────────────────────────────────────────────────
    auto makeText = [&](const string& str, unsigned size,
        sf::Color col, float x, float y) -> sf::Text
        {
            sf::Text t;
            t.setFont(font);
            t.setString(str);
            t.setCharacterSize(size);
            t.setFillColor(col);
            t.setPosition(x, y);
            return t;
        };

    auto makeBtn = [](float x, float y, float w, float h,
        sf::Color col) -> sf::RectangleShape
        {
            sf::RectangleShape r(sf::Vector2f(w, h));
            r.setFillColor(col);
            r.setPosition(x, y);
            return r;
        };

    // ═══════════════════════════════════════════════════════
    //  MAIN MENU
    // ═══════════════════════════════════════════════════════
    auto titleText = makeText("CAR DODGE", 90, sf::Color::Yellow, 530.f, 80.f);

    auto btnPlay = makeBtn(785.f, 280.f, 350.f, 68.f, sf::Color(180, 0, 0));
    auto txtPlay = makeText("PLAY", 38, sf::Color::White, 888.f, 292.f);

    auto btnHS = makeBtn(785.f, 378.f, 350.f, 68.f, sf::Color(180, 0, 0));
    auto txtHS = makeText("HIGH SCORE", 26, sf::Color::White, 798.f, 393.f);

    auto btnOpt = makeBtn(785.f, 476.f, 350.f, 68.f, sf::Color(180, 0, 0));
    auto txtOpt = makeText("OPTIONS", 36, sf::Color::White, 835.f, 490.f);

    auto btnHelp = makeBtn(785.f, 574.f, 350.f, 68.f, sf::Color(180, 0, 0));
    auto txtHelp = makeText("HELP", 38, sf::Color::White, 888.f, 586.f);

    auto btnExit = makeBtn(785.f, 672.f, 350.f, 68.f, sf::Color(180, 0, 0));
    auto txtExit = makeText("EXIT", 38, sf::Color::White, 888.f, 684.f);

    // ═══════════════════════════════════════════════════════
    //  ENTER NAME
    // ═══════════════════════════════════════════════════════
    auto namePrompt = makeText("Enter Your Name:", 40, sf::Color::White, 480.f, 320.f);
    sf::RectangleShape nameBox(sf::Vector2f(700.f, 70.f));
    nameBox.setFillColor(sf::Color(40, 40, 40));
    nameBox.setOutlineColor(sf::Color::Yellow);
    nameBox.setOutlineThickness(3.f);
    nameBox.setPosition(610.f, 430.f);
    sf::Text nameInputText = makeText("", 36, sf::Color::Yellow, 625.f, 444.f);

    auto btnStart = makeBtn(810.f, 560.f, 300.f, 68.f, sf::Color(180, 0, 0));
    auto txtStart = makeText("START", 38, sf::Color::White, 853.f, 572.f);

    // ═══════════════════════════════════════════════════════
    //  IN-GAME HUD
    // ═══════════════════════════════════════════════════════
    sf::Text scoreText = makeText("Score: 0", 36, sf::Color::Black, 65.f, 30.f);
    sf::Text livesText = makeText("Lives: 3", 36, sf::Color::Red, 1500.f, 30.f);
    sf::Text nameHUD = makeText("", 26, sf::Color::Black, 65.f, 80.f);

    // Status bar texts
    sf::Text sbScore = makeText("SCORE: 0", 22, sf::Color::White, 20.f, 1020.f);
    sf::Text sbLives = makeText("LIVES: 3", 22, sf::Color::Red, 320.f, 1020.f);
    sf::Text sbSpeed = makeText("SPEED: 1", 22, sf::Color::Yellow, 620.f, 1020.f);
    sf::Text sbName = makeText("", 22, sf::Color(0, 220, 220), 920.f, 1020.f);

    auto btnPause = makeBtn(870.f, 18.f, 180.f, 52.f, sf::Color(70, 70, 70));
    auto txtPause = makeText("| | PAUSE", 22, sf::Color::White, 882.f, 28.f);

    // ═══════════════════════════════════════════════════════
    //  PAUSE SCREEN
    // ═══════════════════════════════════════════════════════
    auto pauseTitle = makeText("PAUSED", 78, sf::Color::White, 660.f, 140.f);

    auto btnResume = makeBtn(785.f, 310.f, 350.f, 68.f, sf::Color(0, 140, 0));
    auto txtResume = makeText("RESUME", 36, sf::Color::White, 825.f, 322.f);

    auto btnRestartPause = makeBtn(785.f, 410.f, 350.f, 68.f, sf::Color(180, 0, 0));
    auto txtRestartPause = makeText("RESTART", 36, sf::Color::White, 820.f, 422.f);

    auto btnMenuPause = makeBtn(785.f, 510.f, 350.f, 68.f, sf::Color(180, 0, 0));
    auto txtMenuPause = makeText("MAIN MENU", 28, sf::Color::White, 800.f, 525.f);

    auto btnExitPause = makeBtn(785.f, 610.f, 350.f, 68.f, sf::Color(180, 0, 0));
    auto txtExitPause = makeText("EXIT", 36, sf::Color::White, 882.f, 622.f);

    // ═══════════════════════════════════════════════════════
    //  GAME OVER SCREEN
    // ═══════════════════════════════════════════════════════
    auto goTitle = makeText("GAME OVER", 86, sf::Color::Red, 480.f, 100.f);
    sf::Text goScore = makeText("", 42, sf::Color::White, 380.f, 260.f);
    sf::Text goHighScore = makeText("", 38, sf::Color::Yellow, 380.f, 330.f);
    sf::Text goName = makeText("", 32, sf::Color(0, 220, 220), 380.f, 400.f);

    // Top-5 table inside Game Over
    std::vector<sf::Text> goTable; // built dynamically

    auto btnRestartGO = makeBtn(785.f, 700.f, 350.f, 68.f, sf::Color(180, 0, 0));
    auto txtRestartGO = makeText("RESTART", 36, sf::Color::White, 820.f, 712.f);

    auto btnMenuGO = makeBtn(785.f, 800.f, 350.f, 68.f, sf::Color(180, 0, 0));
    auto txtMenuGO = makeText("MAIN MENU", 28, sf::Color::White, 800.f, 815.f);

    // ═══════════════════════════════════════════════════════
    //  OPTIONS  (empty as requested)
    // ═══════════════════════════════════════════════════════
    auto optTitle = makeText("OPTIONS", 76, sf::Color::White, 630.f, 140.f);
    auto optSoon = makeText("Coming Soon...", 38, sf::Color(180, 180, 180), 560.f, 420.f);
    auto btnBackOpt = makeBtn(810.f, 700.f, 300.f, 68.f, sf::Color(180, 0, 0));
    auto txtBackOpt = makeText("BACK", 38, sf::Color::White, 873.f, 712.f);

    // ═══════════════════════════════════════════════════════
    //  HELP SCREEN
    // ═══════════════════════════════════════════════════════
    auto helpTitle = makeText("HELP", 76, sf::Color::White, 720.f, 90.f);
    auto helpL1 = makeText("LEFT / RIGHT arrows  ->  move your car", 26, sf::Color::White, 300.f, 260.f);
    auto helpL2 = makeText("Avoid all enemy cars on the road", 26, sf::Color::White, 300.f, 330.f);
    auto helpL3 = makeText("You have 3 lives total", 26, sf::Color::White, 300.f, 400.f);
    auto helpL4 = makeText("Colliding loses 1 life", 26, sf::Color::White, 300.f, 470.f);
    auto helpL5 = makeText("Speed increases with your score", 26, sf::Color::White, 300.f, 540.f);
    auto helpL6 = makeText("After a hit you get 2 sec safe mode", 26, sf::Color::White, 300.f, 610.f);
    auto helpL7 = makeText("Press ESC or Pause btn to pause", 26, sf::Color::White, 300.f, 680.f);
    auto helpL8 = makeText("Survive as long as you can!", 26, sf::Color::Yellow, 300.f, 760.f);
    auto btnBackHelp = makeBtn(810.f, 870.f, 300.f, 68.f, sf::Color(180, 0, 0));
    auto txtBackHelp = makeText("BACK", 38, sf::Color::White, 873.f, 882.f);

    // ═══════════════════════════════════════════════════════
    //  HIGH SCORE SCREEN
    // ═══════════════════════════════════════════════════════
    auto hsTitle = makeText("HIGH SCORES", 66, sf::Color::Yellow, 540.f, 130.f);
    auto btnBackHS = makeBtn(810.f, 830.f, 300.f, 68.f, sf::Color(180, 0, 0));
    auto txtBackHS = makeText("BACK", 38, sf::Color::White, 873.f, 842.f);
    // Table rows built dynamically in draw section

    // ═══════════════════════════════════════════════════════
    //  GAME LOOP
    // ═══════════════════════════════════════════════════════
    while (window.isOpen())
    {
        float dt = clock.restart().asSeconds();

        // ── Road scroll ──────────────────────────────────────
        if (gameState == PLAYING)
        {
            road.move(0.f, 400.f * dt);
            road2.move(0.f, 400.f * dt);
        }
        if (road.getPosition().y >= 1065.f) road.setPosition(0.f, -1065.f);
        if (road2.getPosition().y >= 1065.f) road2.setPosition(0.f, -1065.f);

        // ── Safe mode timer ───────────────────────────────────
        if (safeMode && safeClock.getElapsedTime().asSeconds() > 2.f)
        {
            safeMode = false;
            car.setColor(sf::Color::White);
        }

        // ── Score & enemy speed ───────────────────────────────
        if (gameState == PLAYING)
        {
            score += (int)(dt * 100.f);
            enemySpeed = 700.f + score * 0.3f;
        }

        // ── Player movement ───────────────────────────────────
        if (gameState == PLAYING)
        {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
                car.move(-playerSpeed * dt, 0.f);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
                car.move(playerSpeed * dt, 0.f);
        }

        // ── Boundaries ───────────────────────────────────────
        if (car.getPosition().x < 40.f)
            car.setPosition(40.f, car.getPosition().y);
        if (car.getPosition().x > 1920.f - 190.f)
            car.setPosition(1920.f - 190.f, car.getPosition().y);

        // ── Safe-mode translucency ────────────────────────────
        if (gameState == PLAYING)
            car.setColor(safeMode ? sf::Color(255, 255, 255, 110) : sf::Color::White);

        // ─────────────────────────────────────────────────────
        //  EVENT LOOP
        // ─────────────────────────────────────────────────────
        sf::Event event;
        while (window.pollEvent(event))
        {
            // ── Window close ──────────────────────────────────
            if (event.type == sf::Event::Closed)
                window.close();

            // ── ESC key ───────────────────────────────────────
            if (event.type == sf::Event::KeyPressed &&
                event.key.code == sf::Keyboard::Escape)
            {
                if (gameState == PLAYING) gameState = PAUSED;
                else if (gameState == PAUSED)  gameState = PLAYING;
                else                           window.close();
            }

            // ── Text input for name ───────────────────────────
            if (gameState == ENTER_NAME && event.type == sf::Event::TextEntered)
            {
                if (event.text.unicode == '\b')
                {
                    if (!playerName.empty()) playerName.pop_back();
                }
                else if (event.text.unicode < 128 &&
                    event.text.unicode != '\r' &&
                    event.text.unicode != '\n')
                {
                    if (playerName.size() < 12)
                        playerName += static_cast<char>(event.text.unicode);
                }
            }

            // ── Mouse click (event-based, fires once per click) ──
            if (event.type == sf::Event::MouseButtonReleased &&
                event.mouseButton.button == sf::Mouse::Left)
            {
                sf::Vector2f mp(static_cast<float>(event.mouseButton.x),
                    static_cast<float>(event.mouseButton.y));

                // ── MAIN MENU ─────────────────────────────────
                if (gameState == MAIN_MENU)
                {
                    if (btnPlay.getGlobalBounds().contains(mp))
                    {
                        playerName = "";
                        gameState = ENTER_NAME;
                    }
                    if (btnHS.getGlobalBounds().contains(mp))
                        gameState = HIGH_SCORE_SCREEN;
                    if (btnOpt.getGlobalBounds().contains(mp))
                        gameState = OPTIONS;
                    if (btnHelp.getGlobalBounds().contains(mp))
                        gameState = HELP;
                    if (btnExit.getGlobalBounds().contains(mp))
                        window.close();
                }

                // ── ENTER NAME ────────────────────────────────
                else if (gameState == ENTER_NAME)
                {
                    if (btnStart.getGlobalBounds().contains(mp) && !playerName.empty())
                    {
                        score = 0; lives = 3; gameOver = false;
                        enemySpeed = 700.f;
                        car.setPosition(1050.f, 615.f);
                        car.setColor(sf::Color::White);
                        safeMode = false;
                        resetEnemies(enemies);
                        clock.restart();
                        gameState = PLAYING;
                    }
                }

                // ── IN-GAME PAUSE BUTTON ──────────────────────
                else if (gameState == PLAYING)
                {
                    if (btnPause.getGlobalBounds().contains(mp))
                        gameState = PAUSED;
                }

                // ── PAUSE SCREEN ──────────────────────────────
                else if (gameState == PAUSED)
                {
                    if (btnResume.getGlobalBounds().contains(mp))
                    {
                        gameState = PLAYING;
                        clock.restart();
                    }
                    if (btnRestartPause.getGlobalBounds().contains(mp))
                    {
                        score = 0; lives = 3; gameOver = false;
                        enemySpeed = 700.f;
                        car.setPosition(1050.f, 615.f);
                        car.setColor(sf::Color::White);
                        safeMode = false;
                        resetEnemies(enemies);
                        clock.restart();
                        gameState = PLAYING;
                    }
                    if (btnMenuPause.getGlobalBounds().contains(mp))
                    {
                        playerName = "";
                        gameState = MAIN_MENU;
                    }
                    if (btnExitPause.getGlobalBounds().contains(mp))
                        window.close();
                }

                // ── GAME OVER ─────────────────────────────────
                else if (gameState == GAME_OVER)
                {
                    if (btnRestartGO.getGlobalBounds().contains(mp))
                    {
                        score = 0; lives = 3; gameOver = false;
                        enemySpeed = 700.f;
                        car.setPosition(1050.f, 615.f);
                        car.setColor(sf::Color::White);
                        safeMode = false;
                        resetEnemies(enemies);
                        clock.restart();
                        gameState = PLAYING;
                    }
                    if (btnMenuGO.getGlobalBounds().contains(mp))
                    {
                        playerName = "";
                        gameState = MAIN_MENU;
                    }
                }

                // ── OPTIONS / HELP / HIGH SCORE  Back buttons ─
                else if (gameState == OPTIONS)
                {
                    if (btnBackOpt.getGlobalBounds().contains(mp))
                        gameState = MAIN_MENU;
                }
                else if (gameState == HELP)
                {
                    if (btnBackHelp.getGlobalBounds().contains(mp))
                        gameState = MAIN_MENU;
                }
                else if (gameState == HIGH_SCORE_SCREEN)
                {
                    if (btnBackHS.getGlobalBounds().contains(mp))
                        gameState = MAIN_MENU;
                }
            } // end MouseButtonReleased
        } // end event loop

        // ─────────────────────────────────────────────────────
        //  ENEMY MOVEMENT & RESPAWN
        // ─────────────────────────────────────────────────────
        if (gameState == PLAYING)
        {
            for (int i = 0; i < (int)enemies.size(); i++)
            {
                enemies[i].move(0.f, enemySpeed * dt);

                if (enemies[i].getPosition().y > 1000.f)
                {
                    int  newLane, randomY;
                    bool ok;
                    do {
                        ok = true;
                        newLane = rand() % 6;
                        randomY = -(rand() % 1200 + 400);
                        for (int j = 0; j < (int)enemies.size(); j++)
                        {
                            if (j != i &&
                                (int)enemies[j].getPosition().x == lanes[newLane] &&
                                abs((int)enemies[j].getPosition().y - randomY) < 900)
                            {
                                ok = false;
                            }
                        }
                    } while (!ok);
                    enemies[i].setPosition((float)lanes[newLane], (float)randomY);
                }
            }

            // ── Collision detection ───────────────────────────
            for (int i = 0; i < (int)enemies.size(); i++)
            {
                sf::FloatRect pBounds = car.getGlobalBounds();
                sf::FloatRect eBounds = enemies[i].getGlobalBounds();
                pBounds.left += 9;  pBounds.width -= 18;
                pBounds.top += 10; pBounds.height -= 16;
                eBounds.left += 9;  eBounds.width -= 18;
                eBounds.top += 10; eBounds.height -= 16;

                if (!safeMode && pBounds.intersects(eBounds))
                {
                    sf::FloatRect overlap;
                    pBounds.intersects(eBounds, overlap);
                    spark.setPosition(overlap.left + overlap.width / 2.f,
                        overlap.top + overlap.height / 2.f);
                    showSpark = true;
                    sparkClock.restart();

                    lives--;

                    if (lives <= 0)
                    {
                        // Save to high score list
                        highScores.push_back({ playerName, score });
                        sort(highScores.begin(), highScores.end(),
                            [](const ScoreEntry& a, const ScoreEntry& b)
                            { return a.score > b.score; });
                        if (highScores.size() > 5)
                            highScores.resize(5);

                        car.setColor(sf::Color::White);
                        safeMode = false;
                        gameOver = true;
                        gameState = GAME_OVER;
                    }
                    else
                    {
                        safeMode = true;
                        safeClock.restart();
                        // Respawn hit enemy at safe position
                        int rLane; bool safePos;
                        do {
                            safePos = true;
                            rLane = rand() % 6;
                            for (int j = 0; j < (int)enemies.size(); j++)
                            {
                                if (j != i &&
                                    (int)enemies[j].getPosition().x == lanes[rLane] &&
                                    abs((int)enemies[j].getPosition().y - (-500)) < 700)
                                {
                                    safePos = false;
                                    rLane = rand() % 6;
                                }
                            }
                        } while (!safePos);
                        enemies[i].setPosition((float)lanes[rLane], -500.f);
                    }
                    break;
                }
            }
        }

        // ─────────────────────────────────────────────────────
        //  UPDATE DYNAMIC TEXT
        // ─────────────────────────────────────────────────────
        int speedLevel = (int)(enemySpeed / 700.f);

        scoreText.setString("Score: " + to_string(score));
        livesText.setString("Lives: " + to_string(lives));
        nameHUD.setString(playerName);
        nameInputText.setString(playerName + "_"); // blinking cursor effect

        // Status bar
        sbScore.setString("SCORE: " + to_string(score));
        sbLives.setString("LIVES: " + to_string(lives));
        sbSpeed.setString("SPEED x" + to_string(speedLevel));
        sbName.setString("PLAYER: " + playerName);

        // Game Over dynamic text
        goScore.setString("Your Score  :  " + to_string(score));
        if (!highScores.empty())
            goHighScore.setString("High Score  :  " + to_string(highScores[0].score));
        goName.setString("Player  :  " + playerName);

        // Build game-over top-5 table
        goTable.clear();
        string medals[5] = { "1.", "2.", "3.", "4.", "5." };
        for (int i = 0; i < (int)highScores.size(); i++)
        {
            string line = medals[i] + "  " +
                highScores[i].name + "  -  " +
                to_string(highScores[i].score);
            sf::Text row = makeText(line, 26,
                i == 0 ? sf::Color::Yellow : sf::Color::White,
                400.f, 480.f + i * 40.f);
            goTable.push_back(row);
        }

        // ─────────────────────────────────────────────────────
        //  DRAW
        // ─────────────────────────────────────────────────────
        window.clear();
        window.draw(road);
        window.draw(road2);

        // ── MAIN MENU ─────────────────────────────────────────
        if (gameState == MAIN_MENU)
        {
            window.draw(overlay);
            window.draw(titleText);
            window.draw(btnPlay);  window.draw(txtPlay);
            window.draw(btnHS);    window.draw(txtHS);
            window.draw(btnOpt);   window.draw(txtOpt);
            window.draw(btnHelp);  window.draw(txtHelp);
            window.draw(btnExit);  window.draw(txtExit);
        }

        // ── ENTER NAME ────────────────────────────────────────
        else if (gameState == ENTER_NAME)
        {
            window.draw(overlay);
            window.draw(namePrompt);
            window.draw(nameBox);
            window.draw(nameInputText);
            window.draw(btnStart); window.draw(txtStart);
        }

        // ── PLAYING ───────────────────────────────────────────
        else if (gameState == PLAYING)
        {
            for (auto& e : enemies) window.draw(e);
            window.draw(car);
            if (showSpark)
            {
                window.draw(spark);
                if (sparkClock.getElapsedTime().asSeconds() > 0.2f)
                    showSpark = false;
            }
            // HUD (top)
            window.draw(scoreText);
            window.draw(livesText);
            window.draw(nameHUD);
            window.draw(btnPause);
            window.draw(txtPause);
            // Status bar (bottom)
            window.draw(statusBar);
            window.draw(sbScore);
            window.draw(sbLives);
            window.draw(sbSpeed);
            window.draw(sbName);
        }

        // ── PAUSED ────────────────────────────────────────────
        else if (gameState == PAUSED)
        {
            for (auto& e : enemies) window.draw(e);
            window.draw(car);
            // Status bar still visible
            window.draw(statusBar);
            window.draw(sbScore);
            window.draw(sbLives);
            window.draw(sbSpeed);
            window.draw(sbName);

            window.draw(overlay);
            window.draw(pauseTitle);
            window.draw(btnResume);       window.draw(txtResume);
            window.draw(btnRestartPause); window.draw(txtRestartPause);
            window.draw(btnMenuPause);    window.draw(txtMenuPause);
            window.draw(btnExitPause);    window.draw(txtExitPause);
        }

        // ── GAME OVER ─────────────────────────────────────────
        else if (gameState == GAME_OVER)
        {
            window.draw(overlay);
            window.draw(goTitle);
            window.draw(goScore);
            window.draw(goHighScore);
            window.draw(goName);
            // Top-5 table
            for (auto& row : goTable) window.draw(row);
            window.draw(btnRestartGO); window.draw(txtRestartGO);
            window.draw(btnMenuGO);    window.draw(txtMenuGO);
        }

        // ── OPTIONS ───────────────────────────────────────────
        else if (gameState == OPTIONS)
        {
            window.draw(overlay);
            window.draw(optTitle);
            window.draw(optSoon);
            window.draw(btnBackOpt); window.draw(txtBackOpt);
        }

        // ── HELP ──────────────────────────────────────────────
        else if (gameState == HELP)
        {
            window.draw(overlay);
            window.draw(helpTitle);
            window.draw(helpL1); window.draw(helpL2);
            window.draw(helpL3); window.draw(helpL4);
            window.draw(helpL5); window.draw(helpL6);
            window.draw(helpL7); window.draw(helpL8);
            window.draw(btnBackHelp); window.draw(txtBackHelp);
        }

        // ── HIGH SCORE SCREEN ─────────────────────────────────
        else if (gameState == HIGH_SCORE_SCREEN)
        {
            window.draw(overlay);
            window.draw(hsTitle);

            if (highScores.empty())
            {
                sf::Text noRecord = makeText("No records yet! Play first.",
                    32, sf::Color(180, 180, 180), 380.f, 380.f);
                window.draw(noRecord);
            }
            else
            {
                string medals2[5] = { "1.", "2.", "3.", "4.", "5." };
                for (int i = 0; i < (int)highScores.size(); i++)
                {
                    string line = medals2[i] + "  " +
                        highScores[i].name + "  -  " +
                        to_string(highScores[i].score);
                    sf::Color col = (i == 0) ? sf::Color::Yellow :
                        (i == 1) ? sf::Color(200, 200, 200) :
                        (i == 2) ? sf::Color(205, 127, 50) :
                        sf::Color::White;
                    sf::Text row = makeText(line, 34, col, 400.f, 280.f + i * 60.f);
                    window.draw(row);
                }
            }
            window.draw(btnBackHS); window.draw(txtBackHS);
        }

        window.display();
    } // end game loop

    return 0;
}