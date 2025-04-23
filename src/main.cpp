#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <ctime>
#include <iostream>
#include <vector>
#include <cmath>

/*----------------------------------------------------Global Variables---------------------------------------------------*/
#define winWidth  800
#define winLength  600
#define tx 150 
#define ty 50
sf::RenderWindow window(sf::VideoMode(winWidth, winLength), "Brick Breaker");
// for menu page
sf::RectangleShape winRect(sf::Vector2f(winWidth, winLength));
sf::Font font;
sf::Text title, shadow1, shadow2, shadow3, startText;
std::vector<sf::CircleShape> dots;
sf::RectangleShape rect;
sf::RectangleShape outerRectangle;
sf::CircleShape upperleftcir, upperRightCir, leftCircle, rightCircle;
sf::Clock gameClock;

sf::Texture buttonTexture;
sf::RectangleShape hover;
sf::Sprite startbutton;
sf::Texture MenubackgroundTexture;
sf::Sprite Menubackground;

// Paddle
const float moveSpeed = 6.f;
sf::Texture paddleTexture;
sf::Sprite paddle;
sf::Texture ballTexture;
sf::Sprite ball;

//Ball
bool startBall = false;
int direction = 4; 
int bricksLeft = 0;
int score = 0;
bool gameWon = false;
bool gameLost = false;
const float ballSpeed = 5.0f;

//Brick
class Brick 
{
    public:
    sf::RectangleShape shape;
    bool isDestroyed = false;
    sf::Color topColor = sf::Color::Red;
    sf::Color bottomColor = sf::Color::Red;

    Brick() : isDestroyed(false) {} 
};

std::vector<Brick> bricks;
const int BRICK_ROWS = 6;
const int BRICK_COLUMNS = 10;
const float BRICK_WIDTH = 74.f;
const float BRICK_HEIGHT = 20.f;
const float BRICK_PADDING = 5.5f;
const float BRICK_TOP_OFFSET = 30.f;

//Game messages
sf::Font font2;
sf::Text scoreText;
sf::Text gameOverText;
sf::Text restartText;

sf::Music backgroundMusic;
sf::SoundBuffer buffer;
sf::Sound sound;
/*----------------------------------------------------Initialization Functions------------------------------------------*/

bool initializeFont() {

    if (font2.loadFromFile("C:/Windows/Fonts/Arial.ttf")) {
        return true;
    }


    if (font2.loadFromFile("arial.ttf")) {
        return true;
    }


    const std::vector<std::string> fontPaths = {
        "fonts/arial.ttf",
        "resources/arial.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSans.ttf"  
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

    if (!initializeFont()) {
        std::cerr << "Failed to initialize font! Text will not be displayed." << std::endl;
    }

    scoreText.setFont(font2);
    scoreText.setCharacterSize(24);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition(20.f, window.getSize().y - 40.f);
    scoreText.setString("Score: 0");


    gameOverText.setFont(font2);
    gameOverText.setCharacterSize(48);
    gameOverText.setFillColor(sf::Color::Yellow);
    gameOverText.setString("YOU WON!");
    gameOverText.setPosition(
        (window.getSize().x - gameOverText.getLocalBounds().width) / 2.f,
        (window.getSize().y - gameOverText.getLocalBounds().height) / 2.f);


    restartText.setFont(font2);
    restartText.setCharacterSize(24);
    restartText.setFillColor(sf::Color::Red);
    restartText.setString("Press R to restart or ESC to quit");
    restartText.setPosition(
        (window.getSize().x - restartText.getLocalBounds().width) / 2.f,
        (window.getSize().y - restartText.getLocalBounds().height) / 2.f + 60);

    /*=======================================Initial Paddle State======================================*/
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

    /*======================================Initial Ball State=========================================*/
    if (!ballTexture.loadFromFile("img/ball.png")) {
        std::cerr << "Failed to load ball texture!" << std::endl;
        return false;
    }
    ballTexture.setSmooth(true);
    ball.setTexture(ballTexture);
    ball.setPosition(window.getSize().x / 2.f, window.getSize().y / 2.f);
    ball.setScale(0.9f, 0.9f);

    /*======================================Initial Bricks State=======================================*/
    bricksLeft = BRICK_ROWS * BRICK_COLUMNS;

    // Create gradient-colored bricks
    for (int row = 0; row < BRICK_ROWS; ++row) {
        for (int col = 0; col < BRICK_COLUMNS; ++col) {
            Brick brick;


            float x = col * (BRICK_WIDTH + BRICK_PADDING) + BRICK_PADDING;
            float y = row * (BRICK_HEIGHT + BRICK_PADDING) + BRICK_TOP_OFFSET;

            brick.shape.setSize(sf::Vector2f(BRICK_WIDTH, BRICK_HEIGHT));
            brick.shape.setPosition(x, y);
            brick.shape.setOutlineThickness(2.f);
            brick.shape.setOutlineColor(sf::Color::Black);


            float hue = (row * 40.f); 
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
/*----------------------------------------------------Start Menu-------------------------------------------------------*/
bool StartMenu() {
    float radius = 25.f;
    float length = 180.f;
    float yPosition = 450.f;
    // Set background color and position
    winRect.setFillColor(sf::Color(97, 132, 170));
    winRect.setPosition(0.f, 0.f);
    // Load and apply font to title text
    font.loadFromFile("Font/VCR_OSD_MONO.ttf");
    // Create drop shadow effects
    title.setFont(font);
    title.setString(" BRICK\nBREAKER");
    title.setCharacterSize(100);
    title.setFillColor(sf::Color(20, 45, 75));
    title.setStyle(sf::Text::Bold);
    title.setOutlineColor(sf::Color(20, 45, 75));
    title.setOutlineThickness(3);
    title.setPosition(tx, ty);
    // Generate background sprinkles/dots
    shadow1 = title; shadow2 = title; shadow3 = title;
    shadow1.setFillColor(sf::Color(194, 231, 255)); shadow1.setPosition(tx + 4, ty + 4);
    shadow2.setFillColor(sf::Color(194, 231, 255)); shadow2.setPosition(tx + 4, ty + 8);
    shadow3.setFillColor(sf::Color(194, 231, 255)); shadow3.setPosition(tx + 4, ty + 10);
    // Setup START button (technically text type) rectangle and surrounding elements
    std::srand(static_cast<unsigned>(std::time(0)));
    for (int i = 0; i < 80; ++i) {
        sf::CircleShape dot(3);
        dot.setFillColor(sf::Color(20, 45, 75));
        float x = static_cast<float>(std::rand() % window.getSize().x);
        float y = static_cast<float>(std::rand() % window.getSize().y);
        dot.setPosition(x, y);
        dots.push_back(dot);
    }
    // Start button shaping
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

    // Staging the start button text
    startText.setFont(font);
    startText.setString("START");
    startText.setCharacterSize(50);
    startText.setFillColor(sf::Color(194, 231, 255));
    startText.setStyle(sf::Text::Bold);
    startText.setLetterSpacing(0.75f);
    startText.setOutlineColor(sf::Color(20, 45, 75));
    startText.setOutlineThickness(3);
    // Center the text inside the button, and compensation for adjustment
    sf::FloatRect textBounds = startText.getLocalBounds();
    startText.setOrigin(textBounds.width / 2, textBounds.height / 2);
    startText.setPosition(260.f + radius + length / 2, (yPosition + radius) - 11);
    // Shape for hover effects
    sf::Cursor Arrow;
    Arrow.loadFromSystem(sf::Cursor::Arrow);
    sf::Cursor Hand;
    Hand.loadFromSystem(sf::Cursor::Hand);
    // Checking if the button is clicked
    sf::Vector2i mousePos = sf::Mouse::getPosition(window); // Get mouse co-ordinates relative to window
    sf::FloatRect buttonbounds = startText.getGlobalBounds();   // Get button corner co-ordinates
    if(buttonbounds.contains(static_cast<sf::Vector2f>(mousePos))){
        window.setMouseCursor(Hand); // Hovering effect Cursor change
        startText.setFillColor(sf::Color(169, 169, 169)); // Text color change hover effect
        if(sf::Mouse::isButtonPressed(sf::Mouse::Left)){
            buffer.loadFromFile("game-sounds/click.ogg"); 
            sound.setBuffer(buffer);// Sound for when the start button is clicked
            sound.setVolume(50);
            sound.play();
            return true;
        }
    }
    else{
        startText.setFillColor(sf::Color(194, 231, 255)); // Resetting the cursor and color when not hovering 
        window.setMouseCursor(Arrow); 
    }
    return false;
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
    //Replay Music
    backgroundMusic.play();
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
                (window.getSize().y - gameOverText.getLocalBounds().height) / 2.f - 40);
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
    // Music settings
    if(!backgroundMusic.openFromFile("Absolute-Class/City-of-Tears.ogg")){
        std::cerr << "Failed to load background music.\n";
    }
    backgroundMusic.setLoop(true);
    backgroundMusic.setVolume(10);
    // Plays Music in Menu
    backgroundMusic.play();
    // Menu loop and render
    window.setFramerateLimit(60);
    // Runs menu till start
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
        for (const auto& dot : dots)
            window.draw(dot);
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
    // Plays Music while the game is in play
    if(!gameLost && !gameWon){
        backgroundMusic.pause();
        if(!backgroundMusic.openFromFile("Absolute-Class/Soul-Sanctum.ogg")){
            std::cerr << "Failed to load background music.\n";
        }
        backgroundMusic.setLoop(true);
        backgroundMusic.setVolume(10);
        backgroundMusic.play();
    }
    else if(gameLost || gameWon){
        backgroundMusic.pause();
    }
    //Game loop
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                backgroundMusic.pause();
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
                    backgroundMusic.pause();
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
            backgroundMusic.pause();
        }

        // Display everything
        window.display();
    } 

    return 0;  
}