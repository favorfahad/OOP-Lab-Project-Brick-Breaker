#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <ctime>
#include <iostream>
#include <vector>
#include <cmath>
#include <memory>

#define winWidth  800
#define winLength 600
#define tx 150 
#define ty 50

// Resource Manager to load fonts and textures
class ResourceManager {
public:
    sf::Font gameFont;
    sf::Texture paddleTexture;
    sf::Texture ballTexture;
    sf::Texture powerupTexture;

    bool loadResources() {
        // Load game font with fallback paths
        if (gameFont.loadFromFile("C:/Windows/Fonts/Arial.ttf") ||
            gameFont.loadFromFile("arial.ttf") ||
            gameFont.loadFromFile("fonts/arial.ttf") ||
            gameFont.loadFromFile("resources/arial.ttf") ||
            gameFont.loadFromFile("/usr/share/fonts/truetype/freefont/FreeSans.ttf")) {
            // Font loaded successfully
        } else {
            std::cerr << "Warning: Couldn't load any font files" << std::endl;
            return false;
        }

        // Load paddle texture
        if (!paddleTexture.loadFromFile("img/paddle.png")) {
            std::cerr << "Failed to load paddle texture!" << std::endl;
            return false;
        }
        paddleTexture.setSmooth(true);

        // Load ball texture
        if (!ballTexture.loadFromFile("img/ball.png")) {
            std::cerr << "Failed to load ball texture!" << std::endl;
            return false;
        }
        ballTexture.setSmooth(true);

        // Load powerup texture
        powerupTexture.loadFromFile("img/Powerup.png");

        return true;
    }
};

// Paddle class
class Paddle {
private:
    sf::Sprite sprite;
    const float moveSpeed = 6.5f;

public:
    Paddle(const sf::Texture& texture) {
        sprite.setTexture(texture);
        sprite.setScale(0.1f, 0.1f);
    }

    void setPosition(float x, float y) { sprite.setPosition(x, y); }
    sf::Vector2f getPosition() const { return sprite.getPosition(); }
    const sf::Sprite& getSprite() const { return sprite; }
    void move(float dx, float dy) { sprite.move(dx, dy); }
    sf::FloatRect getGlobalBounds() const { return sprite.getGlobalBounds(); }
    float getMoveSpeed() const { return moveSpeed; }
};

// Ball class
class Ball {
private:
    sf::Sprite sprite;
    float speed;
    int direction;
    bool startBall;

public:
    Ball(const sf::Texture& texture, float initialSpeed = 5.0f)
        : speed(initialSpeed), direction(4), startBall(false) {
        sprite.setTexture(texture);
        sprite.setScale(0.9f, 0.9f);
    }

    void setPosition(float x, float y) { sprite.setPosition(x, y); }
    const sf::Sprite& getSprite() const { return sprite; }
    void move(float dx, float dy) { sprite.move(dx, dy); }
    sf::FloatRect getGlobalBounds() const { return sprite.getGlobalBounds(); }
    sf::Vector2f getPosition() const { return sprite.getPosition(); }
    bool isStarted() const { return startBall; }
    void setStarted(bool started) { startBall = started; }
    int getDirection() const { return direction; }
    void setDirection(int dir) { direction = dir; }
    float getSpeed() const { return speed; }
    void setSpeed(float s) { speed = s; }

    // Encapsulates handleBallMovement
    void handleBallMovement(sf::RenderWindow& window, bool& gameLost, sf::Text& gameOverText) {
        if (!startBall) return;
        float dx = 0, dy = 0;
        switch (direction) {
            case 1: dx = -speed; dy = -speed; break;
            case 2: dx = speed; dy = -speed; break;
            case 3: dx = -speed; dy = speed; break;
            case 4: dx = speed; dy = speed; break;
        }
        move(dx, dy);

        if (getPosition().x <= 0) setDirection(direction == 1 ? 2 : 4);
        if (getPosition().x + getGlobalBounds().width >= window.getSize().x) setDirection(direction == 2 ? 1 : 3);
        if (getPosition().y <= 0) setDirection(direction == 1 ? 3 : 4);
        if (getPosition().y > window.getSize().y) {
            gameLost = true;
            gameOverText.setString("GAME OVER");
            gameOverText.setPosition(
                (window.getSize().x - gameOverText.getLocalBounds().width) / 2.f,
                (window.getSize().y - gameOverText.getLocalBounds().height) / 2.f
            );
        }
    }

    // Encapsulates checkPaddleCollision
    void checkPaddleCollision(const Paddle& paddle) {
        if (getGlobalBounds().intersects(paddle.getGlobalBounds())) {
            setDirection(direction == 3 ? 1 : 2);
            setPosition(getPosition().x, paddle.getPosition().y - getGlobalBounds().height);
        }
    }
};

// Brick class
class Brick {
public:
    sf::RectangleShape shape;
    bool isDestroyed = false;
    sf::Color topColor = sf::Color::Red;
    sf::Color bottomColor = sf::Color::Red;

    Brick() {
        shape.setSize(sf::Vector2f(74.f, 20.f));
        shape.setOutlineThickness(2.f);
        shape.setOutlineColor(sf::Color::Black);
    }

    void setPosition(float x, float y) { shape.setPosition(x, y); }

    // Encapsulates drawGradientBrick
    void drawGradientBrick(sf::RenderWindow& window) const {
        if (isDestroyed) return;
        sf::Vector2f size = shape.getSize();
        sf::Vector2f position = shape.getPosition();
        sf::VertexArray gradient(sf::Quads, 4);
        gradient[0].position = position;
        gradient[0].color = topColor;
        gradient[1].position = sf::Vector2f(position.x + size.x, position.y);
        gradient[1].color = topColor;
        gradient[2].position = sf::Vector2f(position.x + size.x, position.y + size.y);
        gradient[2].color = bottomColor;
        gradient[3].position = sf::Vector2f(position.x, position.y + size.y);
        gradient[3].color = bottomColor;
        window.draw(gradient);
        window.draw(shape);
    }

    // Encapsulates checkBrickCollisions logic for a single brick
    bool checkCollision(Ball& ball, int& bricksLeft, int& score, sf::Text& scoreText, bool& gameWon, sf::Text& gameOverText) {
        if (isDestroyed) return false;
        if (ball.getGlobalBounds().intersects(shape.getGlobalBounds())) {
            isDestroyed = true;
            bricksLeft--;
            score += 10;
            scoreText.setString("Score: " + std::to_string(score));
            ball.setDirection(ball.getDirection() == 1 ? 3 : (ball.getDirection() == 2 ? 4 : (ball.getDirection() == 3 ? 1 : 2)));
            if (bricksLeft == 0) {
                gameWon = true;
                // Ensure the text is centered horizontally
                gameOverText.setString("YOU WON!");
                gameOverText.setPosition(
                    (winWidth - gameOverText.getGlobalBounds().width) / 2,
                    (winLength - gameOverText.getGlobalBounds().height) / 2
                );
            }
            return true;
        }
        return false;
    }
};

// Powerup base class
class Powerup {
protected:
    sf::Sprite powerup;
    sf::Texture& texture;

public:
    Powerup(sf::Texture& tex) : texture(tex) {
        powerup.setTexture(texture);
        powerup.setScale(0.08f, 0.08f);
        float randomX = static_cast<float>(std::rand() % 600);
        powerup.setPosition(randomX, 250.f);
    }

    void fall() {
        powerup.move(0.f, 1.5f); // to make the powerup move
    }

    void Spawn(sf::RenderWindow& window) {
        window.draw(powerup);
    }

    virtual void Perform() {} // Actually activates the powerup

    bool checkCollision(const sf::FloatRect& paddleBounds) {
        return powerup.getGlobalBounds().intersects(paddleBounds); // Check if the power-up intersects with the paddle
    }

    sf::Vector2f getPosition() const {
        return powerup.getPosition(); // returns position to be used in check
    }
};

// making powerup1 (good)
class DestroyBrickspower : public Powerup {
private:
    std::vector<Brick>& bricks;
    int& bricksLeft;
    int& score;

public:
    DestroyBrickspower(sf::Texture& tex, std::vector<Brick>& brks, int& brksLeft, int& scr)
        : Powerup(tex), bricks(brks), bricksLeft(brksLeft), score(scr) {}

    void Perform() override {
        int size = bricksLeft;
        int removed = 0;
        for (auto it = bricks.end() - 1; it >= bricks.begin(); --it) { // iterating through the vector of Brick
            if (removed == size / 3) { // removing 1/3 of current bricks
                return;
            }
            else {
                if (!it->isDestroyed) {      // updating values of bricks and scoreboard
                    it->isDestroyed = true;
                    bricksLeft--;
                    score += 10;
                    removed++;
                }
            }
        }
    }
};

// making powerup2 (bad)
class SpeedupBallpower : public Powerup {
private:
    Ball& ball;

public:
    SpeedupBallpower(sf::Texture& tex, Ball& b)
        : Powerup(tex), ball(b) {}

    void Perform() override { // speeds up the ball
        ball.setSpeed(ball.getSpeed() + 1.f);
    }
};

// Game class
class Game {
private:
    sf::RenderWindow window;
    ResourceManager resources;
    std::unique_ptr<Paddle> paddle;
    std::unique_ptr<Ball> ball;
    std::vector<Brick> bricks;
    std::vector<Powerup*> activePowerups;
    sf::Text scoreText;
    sf::Text gameOverText;
    sf::Text restartText;
    sf::Text title;
    sf::Text startText;
    sf::RectangleShape winRect;
    sf::RectangleShape outerRectangle;
    sf::RectangleShape rect;
    sf::CircleShape upperleftcir;
    sf::CircleShape upperRightCir;
    sf::CircleShape rightCircle;
    sf::CircleShape leftCircle;
    std::vector<sf::CircleShape> dots;
    sf::RectangleShape shadow1;
    sf::RectangleShape shadow2;
    sf::RectangleShape shadow3;
    sf::Music backgroundMusic;
    sf::SoundBuffer buffer;
    sf::Sound sound;
    sf::Clock gameClock;
    sf::Clock powerupClock;

    // Game state
    int bricksLeft = 0;
    int score = 0;
    bool gameWon = false;
    bool gameLost = false;

    // Brick constants
    static constexpr int BRICK_ROWS = 6;
    static constexpr int BRICK_COLUMNS = 10;
    static constexpr float BRICK_WIDTH = 74.f;
    static constexpr float BRICK_HEIGHT = 20.f;
    static constexpr float BRICK_PADDING = 5.5f;
    static constexpr float BRICK_TOP_OFFSET = 30.f;

public:
    Game() : window(sf::VideoMode(winWidth, winLength), "Brick Breaker") {}

    bool initializeSprites() {
        if (!resources.loadResources()) {
            std::cerr << "Failed to initialize resources!" << std::endl;
            return false;
        }

        // Initialize paddle
        paddle = std::make_unique<Paddle>(resources.paddleTexture);
        float paddleX = (window.getSize().x - paddle->getGlobalBounds().width) / 2.f;
        float paddleY = window.getSize().y - paddle->getGlobalBounds().height - 10.f;
        paddle->setPosition(paddleX, paddleY);

        // Initialize ball
        ball = std::make_unique<Ball>(resources.ballTexture);
        ball->setPosition(window.getSize().x / 2.f, window.getSize().y / 2.f);

        // Initialize UI texts
        scoreText.setFont(resources.gameFont);
        scoreText.setCharacterSize(24);
        scoreText.setFillColor(sf::Color::Black);
        scoreText.setPosition(20.f, window.getSize().y - 40.f);
        scoreText.setString("Score: 0");

        gameOverText.setFont(resources.gameFont);
        gameOverText.setCharacterSize(48);
        gameOverText.setFillColor(sf::Color::Yellow);
        gameOverText.setString("YOU WON!");
        gameOverText.setPosition(
            (window.getSize().x - gameOverText.getLocalBounds().width) / 2.f,
            (window.getSize().y - gameOverText.getLocalBounds().height) / 2.f
        );

        restartText.setFont(resources.gameFont);
        restartText.setCharacterSize(24);
        restartText.setFillColor(sf::Color::Red);
        restartText.setString("Press R to restart or ESC to quit");
        restartText.setPosition(
            (window.getSize().x - restartText.getLocalBounds().width) / 2.f,
            (window.getSize().y - restartText.getLocalBounds().height) / 2.f + 60
        );

        // Initialize bricks
        bricks.clear();
        bricksLeft = BRICK_ROWS * BRICK_COLUMNS;
        for (int row = 0; row < BRICK_ROWS; ++row) {
            for (int col = 0; col < BRICK_COLUMNS; ++col) {
                Brick brick;
                float x = col * (BRICK_WIDTH + BRICK_PADDING) + BRICK_PADDING;
                float y = row * (BRICK_HEIGHT + BRICK_PADDING) + BRICK_TOP_OFFSET;
                brick.setPosition(x, y);

                float hue = (row * 40.f);
                brick.topColor = sf::Color(
                    static_cast<sf::Uint8>(255 * (0.7f + 0.3f * std::sin(hue * 3.14159265f / 180.f))),
                    static_cast<sf::Uint8>(255 * (0.7f + 0.3f * std::sin((hue + 120) * 3.14159265f / 180.f))),
                    static_cast<sf::Uint8>(255 * (0.7f + 0.3f * std::sin((hue + 240) * 3.14159265f / 180.f)))
                );
                brick.bottomColor = sf::Color(
                    static_cast<sf::Uint8>(brick.topColor.r * 0.7f),
                    static_cast<sf::Uint8>(brick.topColor.g * 0.7f),
                    static_cast<sf::Uint8>(brick.topColor.b * 0.7f)
                );
                bricks.push_back(brick);
            }
        }

        // Initialize menu elements
        winRect.setSize(sf::Vector2f(winWidth, winLength));
        winRect.setFillColor(sf::Color(97, 132, 170));
        winRect.setPosition(0.f, 0.f);

        if (!resources.gameFont.loadFromFile("Font/VCR_OSD_MONO.ttf")) {
            std::cerr << "Failed to load menu font!" << std::endl;
            return false;
        }

        title.setFont(resources.gameFont);
        title.setString(" BRICK\nBREAKER");
        title.setCharacterSize(100);
        title.setFillColor(sf::Color(20, 45, 75));
        title.setStyle(sf::Text::Bold);
        title.setOutlineColor(sf::Color(20, 45, 75));
        title.setOutlineThickness(3);
        title.setPosition(tx, ty);

        std::srand(static_cast<unsigned>(std::time(0)));
        for (int i = 0; i < 80; ++i) {
            sf::CircleShape dot(3);
            dot.setFillColor(sf::Color(20, 45, 75));
            float x = static_cast<float>(std::rand() % winWidth);
            float y = static_cast<float>(std::rand() % winLength);
            dot.setPosition(x, y);
            dots.push_back(dot);
        }

        float radius = 25.f;
        float length = 180.f;
        float yPosition = 450.f;
        float shiftLeft = -50.f;
        rect.setSize(sf::Vector2f(length, 2 * radius));
        rect.setFillColor(sf::Color(20, 45, 75));
        rect.setPosition(310.f + radius + shiftLeft, yPosition);

        outerRectangle.setSize(sf::Vector2f(length, (2 * radius) + 4));
        outerRectangle.setFillColor(sf::Color(194, 231, 255));
        outerRectangle.setPosition(310.f + radius + shiftLeft, yPosition - 2);

        upperleftcir.setRadius(radius + 2);
        upperleftcir.setFillColor(sf::Color(194, 231, 255));
        upperleftcir.setPosition(305.f + shiftLeft, yPosition - 1.97f);

        leftCircle.setRadius(radius);
        leftCircle.setFillColor(sf::Color(20, 45, 75));
        leftCircle.setPosition(307.f + shiftLeft, yPosition);

        upperRightCir.setRadius(radius + 2);
        upperRightCir.setFillColor(sf::Color(194, 231, 255));
        upperRightCir.setPosition(308.f + length + shiftLeft, yPosition - 1.97f);

        rightCircle.setRadius(radius);
        rightCircle.setFillColor(sf::Color(20, 45, 75));
        rightCircle.setPosition(310.f + length + shiftLeft, yPosition);

        startText.setFont(resources.gameFont);
        startText.setString("START");
        startText.setCharacterSize(50);
        startText.setFillColor(sf::Color(194, 231, 255));
        startText.setStyle(sf::Text::Bold);
        startText.setLetterSpacing(0.75f);
        startText.setOutlineColor(sf::Color(20, 45, 75));
        startText.setOutlineThickness(3);
        sf::FloatRect textBounds = startText.getLocalBounds();
        startText.setOrigin(textBounds.width / 2, textBounds.height / 2);
        startText.setPosition(260.f + radius + length / 2, (yPosition + radius) - 11);

        return true;
    }

    // Updated StartMenu to handle mouse input and sound
    bool StartMenu() {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::FloatRect buttonBounds = startText.getGlobalBounds();
        sf::Cursor Arrow;
        Arrow.loadFromSystem(sf::Cursor::Arrow);
        sf::Cursor Hand;
        Hand.loadFromSystem(sf::Cursor::Hand);

        if (buttonBounds.contains(static_cast<sf::Vector2f>(mousePos))) {
            window.setMouseCursor(Hand);
            startText.setFillColor(sf::Color(169, 169, 169));
            if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                buffer.loadFromFile("game-sounds/click.ogg");
                sound.setBuffer(buffer);
                sound.setVolume(50);
                sound.play();
                return true;
            }
        } else {
            startText.setFillColor(sf::Color(194, 231, 255));
            window.setMouseCursor(Arrow);
        }

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed || (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)) {
                backgroundMusic.pause();
                window.close();
                return true;
            }
        }
        return false;
    }

    Powerup* createpowerup() {
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        int chance = std::rand() % 100;
        if (chance <= 40) {
            return new DestroyBrickspower(resources.powerupTexture, bricks, bricksLeft, score);
        } else {
            return new SpeedupBallpower(resources.powerupTexture, *ball);
        }
    }

    void resetGame() {
        // Clear existing state
        bricks.clear();
        for (auto* pu : activePowerups) delete pu;
        activePowerups.clear();

        // Reinitialize game objects
        ball->setStarted(false);
        ball->setDirection(4);
        ball->setSpeed(5.0f);
        ball->setPosition(window.getSize().x / 2.f, window.getSize().y / 2.f);

        float paddleX = (window.getSize().x - paddle->getGlobalBounds().width) / 2.f;
        float paddleY = window.getSize().y - paddle->getGlobalBounds().height - 10.f;
        paddle->setPosition(paddleX, paddleY);

        // Reinitialize bricks
        bricksLeft = BRICK_ROWS * BRICK_COLUMNS;
        for (int row = 0; row < BRICK_ROWS; ++row) {
            for (int col = 0; col < BRICK_COLUMNS; ++col) {
                Brick brick;
                float x = col * (BRICK_WIDTH + BRICK_PADDING) + BRICK_PADDING;
                float y = row * (BRICK_HEIGHT + BRICK_PADDING) + BRICK_TOP_OFFSET;
                brick.setPosition(x, y);

                float hue = (row * 40.f);
                brick.topColor = sf::Color(
                    static_cast<sf::Uint8>(255 * (0.7f + 0.3f * std::sin(hue * 3.14159265f / 180.f))),
                    static_cast<sf::Uint8>(255 * (0.7f + 0.3f * std::sin((hue + 120) * 3.14159265f / 180.f))),
                    static_cast<sf::Uint8>(255 * (0.7f + 0.3f * std::sin((hue + 240) * 3.14159265f / 180.f)))
                );
                brick.bottomColor = sf::Color(
                    static_cast<sf::Uint8>(brick.topColor.r * 0.7f),
                    static_cast<sf::Uint8>(brick.topColor.g * 0.7f),
                    static_cast<sf::Uint8>(brick.topColor.b * 0.7f)
                );
                bricks.push_back(brick);
            }
        }

        // Reset game state
        score = 0;
        scoreText.setString("Score: 0");
        gameWon = false;
        gameLost = false;
        powerupClock.restart();
        backgroundMusic.play();
    }

    void run() {
        window.setFramerateLimit(60);
        if (!initializeSprites()) {
            std::cerr << "Initialization failed!" << std::endl;
            return;
        }

        // Music for menu
        if (!backgroundMusic.openFromFile("Absolute-Class/Soul-Sanctum.ogg")) {
            std::cerr << "Failed to load background music.\n";
        }
        backgroundMusic.setLoop(true);
        backgroundMusic.setVolume(10);
        backgroundMusic.play();

        // Menu loop
        while (!StartMenu()) {
            // Animate title
            float time = gameClock.getElapsedTime().asSeconds();
            float offsetY = std::sin(time * 2) * 10;
            title.setPosition(tx, ty + offsetY);
            sf::Event event;
            while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed){
                backgroundMusic.pause();
                window.close();
            }
            if (event.key.code == sf::Keyboard::Escape) {
                backgroundMusic.pause();
                window.close();
            }
        }
            // Render Menu Page
            window.clear(sf::Color::Black);
            window.draw(winRect);
            window.draw(upperleftcir);
            window.draw(upperRightCir);
            window.draw(rightCircle);
            window.draw(leftCircle);
            window.draw(outerRectangle);
            window.draw(rect);
            for (const auto& dot : dots) window.draw(dot);
            window.draw(shadow1);
            window.draw(shadow2);
            window.draw(shadow3);
            window.draw(title);
            window.draw(startText);
            window.display();
        }

        // Resetting the cursor state
        sf::Cursor Arrow;
        Arrow.loadFromSystem(sf::Cursor::Arrow);
        window.setMouseCursor(Arrow);

        // Main Game music
        backgroundMusic.pause();
        if (!backgroundMusic.openFromFile("Absolute-Class/City-of-Tears.ogg")) {
            std::cerr << "Failed to load game music.\n";
        }
        backgroundMusic.play();

        // Main game loop
        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    window.close();
                }
                if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::Space && !ball->isStarted() && !gameWon && !gameLost) {
                        ball->setStarted(true);
                    }
                    if ((gameWon || gameLost) && event.key.code == sf::Keyboard::R) {
                        resetGame();
                    }
                    if (event.key.code == sf::Keyboard::Escape) {
                        window.close();
                    }
                }
            }

            // Game logic
            if (!gameWon && !gameLost) {
                // Handle input and movement
                if ((sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) &&
                    paddle->getPosition().x > 5) {
                    paddle->move(-paddle->getMoveSpeed(), 0);
                    if (!ball->isStarted()) ball->move(-paddle->getMoveSpeed(), 0);
                }
                if ((sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) &&
                    paddle->getPosition().x + paddle->getGlobalBounds().width < window.getSize().x) {
                    paddle->move(paddle->getMoveSpeed(), 0);
                    if (!ball->isStarted()) ball->move(paddle->getMoveSpeed(), 0);
                }
                ball->handleBallMovement(window, gameLost, gameOverText);
                ball->checkPaddleCollision(*paddle);
                bool collisionProcessed = false;
                for (auto& brick : bricks) {
                    if (!collisionProcessed && brick.checkCollision(*ball, bricksLeft, score, scoreText, gameWon, gameOverText)) {
                        collisionProcessed = true; // Only process one collision per frame
                    }
                }
                // Power-up spawning with timing of every 10 seconds only when the ball starts to move
                if((ball->isStarted())){
                    float powerupTime = powerupClock.getElapsedTime().asSeconds();
                    if (powerupTime >= 10.f && activePowerups.empty()) { // creates a powerup when no other is present
                        Powerup* newPU = createpowerup(); 
                        if (newPU) {
                            activePowerups.push_back(newPU);
                            powerupClock.restart(); // restart the clock for another 10 second count
                        }
                    }
                }
                else{
                    powerupClock.restart(); // reset the clock when the ball is not moving
                }
                // Update power-ups to make them fall and delete them eventually
                for (auto it = activePowerups.begin(); it != activePowerups.end(); ) {
                    (*it)->fall(); // makes the pu fall
                    if ((*it)->checkCollision(paddle->getGlobalBounds())) { // checks for collision with the paddle
                        (*it)->Perform(); // the pu performs its action
                        delete *it;
                        it = activePowerups.erase(it); // deletes pu once it's used
                    }
                    else if ((*it)->getPosition().y > window.getSize().y) {
                        delete *it;
                        it = activePowerups.erase(it);
                    }
                    else {
                        ++it;
                    }
                }
            }

            // Rendering
            window.clear(sf::Color(30, 30, 40));
            // Draw game objects in correct order
            for (const auto& brick : bricks) {
                brick.drawGradientBrick(window);
            }
            window.draw(paddle->getSprite());
            window.draw(ball->getSprite());
            // Draw power-ups LAST so they appear on top and a condition to only spawn them only when game is running
            if (!gameLost && !gameWon) {
                for (auto& pu : activePowerups) {
                    pu->Spawn(window);
                }
            }
            // Draw UI
            window.draw(scoreText);
            if (gameWon || gameLost) { // to make sure to delete the powerups once game ends and display relevant texts
                backgroundMusic.pause();
                sf::RectangleShape overlay(sf::Vector2f(window.getSize().x, window.getSize().y));
                overlay.setFillColor(sf::Color(0, 0, 0, 150));
                for (auto* pu : activePowerups) {
                    delete pu;
                }
                activePowerups.clear();
                window.draw(overlay);
                window.draw(gameOverText);
                window.draw(restartText);
            }
            window.display();
        }

        // Cleanup
        for (auto* pu : activePowerups) {
            delete pu; // makes sure there is no data leakage
        }
        activePowerups.clear(); // clears the vector of powerups
    }
};

//-------------------------------------------------------Main Loop------------------------------------------------------/
int main() {
    Game game;
    game.run();
    return 0;
}