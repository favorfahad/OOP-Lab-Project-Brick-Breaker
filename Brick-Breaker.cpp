#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cmath>

/*----------------------------------------------------Global Variables---------------------------------------------------*/
sf::RenderWindow window(sf::VideoMode(1000, 800), "Breakout Game");
const float moveSpeed = 5.3f;
sf::Texture paddleTexture;
sf::Sprite paddle;
sf::Texture ballTexture;
sf::Sprite ball;

// Game state variables
bool startBall = false;
int direction = 4; // 1: Top Right, 2: Top Left, 3: Bottom Left, 4: Bottom Right
int bricksLeft = 0;
int score = 0;
bool gameWon = false;
bool gameLost = false;
const float ballSpeed = 5.0f;

// Brick-related variables
struct Brick 
{
    sf::RectangleShape shape;
    bool isDestroyed = false;
    sf::Color topColor = sf::Color::Red;
    sf::Color bottomColor = sf::Color::Red;

    Brick() : isDestroyed(false) {} // Constructor to ensure initialization
};

std::vector<Brick> bricks;
const int BRICK_ROWS = 6;
const int BRICK_COLUMNS = 10;
const float BRICK_WIDTH = 90.f;
const float BRICK_HEIGHT = 30.f;
const float BRICK_PADDING = 10.f;
const float BRICK_TOP_OFFSET = 50.f;

// Text display
sf::Font font;
sf::Text scoreText;
sf::Text gameOverText;
sf::Text restartText;

/*----------------------------------------------------Initialization Functions------------------------------------------*/

bool initializeFont() {
    // Try loading from Windows fonts first
    if (font.loadFromFile("C:/Windows/Fonts/Arial.ttf")) {
        return true;
    }

    // Then try bundled font
    if (font.loadFromFile("arial.ttf")) {
        return true;
    }

    // Try common alternative locations
    const std::vector<std::string> fontPaths = {
        "fonts/arial.ttf",
        "resources/arial.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSans.ttf"  // Linux fallback
    };

    for (const auto& path : fontPaths) {
        if (font.loadFromFile(path)) {
            return true;
        }
    }

    std::cerr << "Warning: Couldn't load any font files" << std::endl;
    return false;
}

bool initializeSprites() {
    // Initialize font
    if (!initializeFont()) {
        std::cerr << "Failed to initialize font! Text will not be displayed." << std::endl;
    }

    // Set up score text
    scoreText.setFont(font);
    scoreText.setCharacterSize(24);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition(20.f, window.getSize().y - 40.f);
    scoreText.setString("Score: 0");

    // Set up game over text
    gameOverText.setFont(font);
    gameOverText.setCharacterSize(48);
    gameOverText.setFillColor(sf::Color::Yellow);
    gameOverText.setString("YOU WON!");
    gameOverText.setPosition(
        (window.getSize().x - gameOverText.getLocalBounds().width) / 2.f,
        (window.getSize().y - gameOverText.getLocalBounds().height) / 2.f
    );

    // Set up restart instructions text
    restartText.setFont(font);
    restartText.setCharacterSize(24);
    restartText.setFillColor(sf::Color::White);
    restartText.setString("Press R to restart or ESC to quit");
    restartText.setPosition(
        (window.getSize().x - restartText.getLocalBounds().width) / 2.f,
        (window.getSize().y - restartText.getLocalBounds().height) / 2.f + 60
    );

    /*=======================================Paddle ======================================*/
    if (!paddleTexture.loadFromFile("img/paddle.png")) {
        std::cerr << "Failed to load paddle texture!" << std::endl;
        return false;
    }
    paddleTexture.setSmooth(true);
    paddle.setTexture(paddleTexture);
    paddle.setScale(0.1f, 0.1f);

    float paddleX = (window.getSize().x - paddle.getGlobalBounds().width) / 2.f;
    float paddleY = window.getSize().y - paddle.getGlobalBounds().height - 10.f;
    paddle.setPosition(paddleX, paddleY);

    /*======================================Ball=========================================*/
    if (!ballTexture.loadFromFile("img/ball.png")) {
        std::cerr << "Failed to load ball texture!" << std::endl;
        return false;
    }
    ballTexture.setSmooth(true);
    ball.setTexture(ballTexture);
    ball.setPosition(window.getSize().x / 2.f, window.getSize().y / 2.f);
    ball.setScale(0.9f, 0.9f);

    /*======================================Bricks=======================================*/
    bricksLeft = BRICK_ROWS * BRICK_COLUMNS;

    // Create gradient-colored bricks
    for (int row = 0; row < BRICK_ROWS; ++row) {
        for (int col = 0; col < BRICK_COLUMNS; ++col) {
            Brick brick;

            // Calculate position
            float x = col * (BRICK_WIDTH + BRICK_PADDING) + BRICK_PADDING;
            float y = row * (BRICK_HEIGHT + BRICK_PADDING) + BRICK_TOP_OFFSET;

            // Set up the brick shape
            brick.shape.setSize(sf::Vector2f(BRICK_WIDTH, BRICK_HEIGHT));
            brick.shape.setPosition(x, y);
            brick.shape.setOutlineThickness(2.f);
            brick.shape.setOutlineColor(sf::Color::Black);

            // Create gradient colors based on row
            float hue = (row * 40.f); // Vary hue by row (0-200 range)
            brick.topColor = sf::Color(
                static_cast<sf::Uint8>(255 * (0.7f + 0.3f * sin(hue * 3.14159265f / 180.f))),
                static_cast<sf::Uint8>(255 * (0.7f + 0.3f * sin((hue + 120) * 3.14159265f / 180.f))),
                static_cast<sf::Uint8>(255 * (0.7f + 0.3f * sin((hue + 240) * 3.14159265f / 180.f)))
            );

            brick.bottomColor = sf::Color(
                static_cast<sf::Uint8>(brick.topColor.r * 0.7f),
                static_cast<sf::Uint8>(brick.topColor.g * 0.7f),
                static_cast<sf::Uint8>(brick.topColor.b * 0.7f)
            );

            bricks.push_back(brick);
        }
    }

    return true;
}

/*----------------------------------------------------Game Logic Functions---------------------------------------------*/

void resetGame() {
    // Reset ball
    ball.setPosition(window.getSize().x / 2.f, window.getSize().y / 2.f);
    startBall = false;
    direction = 4;

    // Reset paddle
    float paddleX = (window.getSize().x - paddle.getGlobalBounds().width) / 2.f;
    float paddleY = window.getSize().y - paddle.getGlobalBounds().height - 10.f;
    paddle.setPosition(paddleX, paddleY);

    // Reset bricks
    for (auto& brick : bricks) {
        brick.isDestroyed = false;
    }
    bricksLeft = BRICK_ROWS * BRICK_COLUMNS;
    score = 0;
    gameWon = false;
    gameLost = false;
}

void handleBallMovement() {
    if (!startBall) return;

    sf::Vector2f ballPos = ball.getPosition();
    sf::FloatRect ballBounds = ball.getGlobalBounds();

    // Ball movement based on direction
    switch (direction) {
    case 1: // Top right
        ball.move(ballSpeed, -ballSpeed);
        if (ballPos.x + ballBounds.width > window.getSize().x) direction = 2;
        if (ballPos.y < 0) direction = 4;
        break;

    case 2: // Top left
        ball.move(-ballSpeed, -ballSpeed);
        if (ballPos.x < 0) direction = 1;
        if (ballPos.y < 0) direction = 3;
        break;

    case 3: // Bottom left
        ball.move(-ballSpeed, ballSpeed);
        if (ballPos.x < 0) direction = 4;
        if (ballPos.y + ballBounds.height > window.getSize().y) {
            gameLost = true;
            gameOverText.setString("GAME OVER");
            gameOverText.setPosition(
                (window.getSize().x - gameOverText.getLocalBounds().width) / 2.f,
                (window.getSize().y - gameOverText.getLocalBounds().height) / 2.f
            );
        }
        break;

    case 4: // Bottom right
        ball.move(ballSpeed, ballSpeed);
        if (ballPos.x + ballBounds.width > window.getSize().x) direction = 3;
        if (ballPos.y + ballBounds.height > window.getSize().y) {
            gameLost = true;
            gameOverText.setString("GAME OVER");
            gameOverText.setPosition(
                (window.getSize().x - gameOverText.getLocalBounds().width) / 2.f,
                (window.getSize().y - gameOverText.getLocalBounds().height) / 2.f
            );
        }
        break;
    }
}

void checkPaddleCollision() {
    sf::FloatRect ballBounds = ball.getGlobalBounds();
    sf::FloatRect paddleBounds = paddle.getGlobalBounds();

    if (ballBounds.intersects(paddleBounds)) {
        // Ball hit the paddle
        if (direction == 3) {
            direction = 2; // Bottom left -> Top left
        }
        else if (direction == 4) {
            direction = 1; // Bottom right -> Top right
        }

        // Add some randomness to the bounce
        float hitPosition = (ball.getPosition().x + ballBounds.width / 2) - paddle.getPosition().x;
        float normalizedHit = hitPosition / paddleBounds.width;

        if (normalizedHit < 0.3f) {
            direction = 2; // Favor left direction
        }
        else if (normalizedHit > 0.7f) {
            direction = 1; // Favor right direction
        }
    }
}

void checkBrickCollisions() {
    sf::FloatRect ballBounds = ball.getGlobalBounds();

    for (auto& brick : bricks) {
        if (!brick.isDestroyed && ballBounds.intersects(brick.shape.getGlobalBounds())) {
            brick.isDestroyed = true;
            bricksLeft--;
            score += 10;

            // Update score display
            scoreText.setString("Score: " + std::to_string(score));

            // Change direction based on where the ball hit the brick
            sf::FloatRect brickBounds = brick.shape.getGlobalBounds();
            float ballCenterX = ball.getPosition().x + ballBounds.width / 2;
            float ballCenterY = ball.getPosition().y + ballBounds.height / 2;

            // Determine collision side
            bool fromLeft = ballCenterX < brickBounds.left;
            bool fromRight = ballCenterX > brickBounds.left + brickBounds.width;
            bool fromTop = ballCenterY < brickBounds.top;
            bool fromBottom = ballCenterY > brickBounds.top + brickBounds.height;

            if (fromLeft || fromRight) {
                // Horizontal bounce
                if (direction == 1) direction = 2;
                else if (direction == 2) direction = 1;
                else if (direction == 3) direction = 4;
                else if (direction == 4) direction = 3;
            }
            else {
                // Vertical bounce
                if (direction == 1) direction = 4;
                else if (direction == 2) direction = 3;
                else if (direction == 3) direction = 2;
                else if (direction == 4) direction = 1;
            }

            break; // Only process one collision per frame
        }
    }

    if (bricksLeft == 0) {
        gameWon = true;
    }
}

void drawGradientBrick(sf::RenderWindow& window, const Brick& brick) {
    if (brick.isDestroyed) return;

    const sf::RectangleShape& shape = brick.shape;
    sf::Vector2f size = shape.getSize();
    sf::Vector2f position = shape.getPosition();

    // Create a vertex array for gradient
    sf::VertexArray gradient(sf::Quads, 4);

    // Top-left
    gradient[0].position = position;
    gradient[0].color = brick.topColor;

    // Top-right
    gradient[1].position = sf::Vector2f(position.x + size.x, position.y);
    gradient[1].color = brick.topColor;

    // Bottom-right
    gradient[2].position = sf::Vector2f(position.x + size.x, position.y + size.y);
    gradient[2].color = brick.bottomColor;

    // Bottom-left
    gradient[3].position = sf::Vector2f(position.x, position.y + size.y);
    gradient[3].color = brick.bottomColor;

    // Draw the gradient
    window.draw(gradient);

    // Draw the outline
    window.draw(shape);
}

/*-------------------------------------------------------Main Loop------------------------------------------------------*/
int main() {
    window.setFramerateLimit(60);

    if (!initializeSprites()) {
        return -1;
    }

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            // Handle key presses
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Space && !startBall && !gameWon && !gameLost) {
                    startBall = true;
                }

                if ((gameWon || gameLost) && event.key.code == sf::Keyboard::R) {
                    resetGame();
                }

                if (event.key.code == sf::Keyboard::Escape) {
                    window.close();
                }
            }
        }

        // Game logic only if game is active
        if (!gameWon && !gameLost) {
            // Paddle movement
            if ((sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) &&
                paddle.getPosition().x > 5) {
                paddle.move(-moveSpeed, 0);
                if (!startBall) {
                    ball.move(-moveSpeed, 0);
                }
            }

            if ((sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) &&
                paddle.getPosition().x + paddle.getGlobalBounds().width < window.getSize().x) {
                paddle.move(moveSpeed, 0);
                if (!startBall) {
                    ball.move(moveSpeed, 0);
                }
            }

            // Game logic
            handleBallMovement();
            checkPaddleCollision();
            checkBrickCollisions();
        }

        // Clear the window
        window.clear(sf::Color(30, 30, 40));

        // Draw all bricks
        for (const auto& brick : bricks) {
            drawGradientBrick(window, brick);
        }

        // Draw paddle and ball
        window.draw(paddle);
        window.draw(ball);

        // Draw UI elements
        window.draw(scoreText);

        // Draw game over message if needed
        if (gameWon || gameLost) {
            sf::RectangleShape overlay(sf::Vector2f(window.getSize().x, window.getSize().y));
            overlay.setFillColor(sf::Color(0, 0, 0, 150));
            window.draw(overlay);
            window.draw(gameOverText);
            window.draw(restartText);
        }

        // Display everything
        window.display();
    }

    return 0;
}

