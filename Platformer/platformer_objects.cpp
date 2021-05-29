#include <SFML/Graphics.hpp>
#include <memory>
#include <time.h>

sf::VideoMode desktop = sf::VideoMode().getDesktopMode();
sf::Vector2f scaleDesktop = sf::Vector2f(desktop.width / 1920.0, desktop.height / 1080.0);

class Background: public sf::Sprite{ //background scene
private:
    sf::Texture texture;
public:
    Background(){
        setPosition(0, 0);
        scale(scaleDesktop);
        srand(time(NULL));
        int random = (rand()% 3) + 1; //generate one of three backgrounds
        texture.loadFromFile("res/background" + std::to_string(random) + ".png");
        setTexture(texture);
    }
};

class Ground: public sf::Sprite{ //ground at the bottom of scene
private:
    sf::Texture texture;
public:
    Ground(const int &fromX, const int &toX){
        scale(scaleDesktop);
        texture.loadFromFile("res/ground.png");
        texture.setRepeated(true);
        setTextureRect(sf::IntRect(0, 0, (toX - fromX), 220)); //220 is texture height in pixels
        setTexture(texture);
        setPosition(fromX * scaleDesktop.x, desktop.height - getGlobalBounds().height + 1);
    }
};

class Platform: public sf::Sprite{ //static platform
private:
    sf::Texture texture;
protected:
    int x, y;
public:
    Platform(const int &x_, const int &y_, const double &scaleWidth): x(x_), y(y_){
        scale(scaleDesktop.x * scaleWidth, scaleDesktop.y);
        texture.loadFromFile("res/platform.png");
        setTexture(texture);
        setPosition(x * scaleDesktop.x, y * scaleDesktop.y);
    }
};

class MovingPlatform: public Platform{ //moving platform
private:
    int fromX, fromY;
    int toX, toY;
public:
    int speedX, speedY;
    MovingPlatform(const int &startX_, const int &startY_, const double &scaleWidth_, const int &fromX_, const int &fromY_, const int &toX_, const int &toY_, const int &speedX_, const int &speedY_):
        Platform(startX_, startY_, scaleWidth_), fromX(fromX_ * scaleDesktop.x), fromY(fromY_ * scaleDesktop.y), toX(toX_ * scaleDesktop.x), toY(toY_ * scaleDesktop.y),
                    speedX(speedX_ * scaleDesktop.x), speedY(speedY_ * scaleDesktop.y){
    }
    void movePlatform(const double &frameTime){
        sf::FloatRect bounds = getGlobalBounds();
        if((bounds.left < fromX && speedX < 0) || (bounds.left > toX && speedX > 0)){
            speedX *= -1;
        }
        if((bounds.top < fromY && speedY < 0) || (bounds.top > toY && speedY > 0)){
            speedY *= -1;
        }
        move(speedX * frameTime, speedY * frameTime);
    }
};

class Coin: public sf::Sprite{
private:
    std::vector<sf::IntRect> frames;
    sf::Texture texture;
    int FPS = 7; //FPS of coin animation
    size_t actualFrame;
    double timeCounter;
public:
    Coin(const int &x, const int&y){
        setPosition(x * scaleDesktop.x, y * scaleDesktop.y);
        scale(0.2 * scaleDesktop.x, 0.2 * scaleDesktop.x);
        texture.loadFromFile("res/coin.png");
        for(int i = 0; i < 6; i++){ //load all animation frames
            frames.emplace_back(sf::IntRect(i*325, 0, 325, 325));
        }
        actualFrame = 0;
        setTextureRect(sf::IntRect(frames[actualFrame]));
        setTexture(texture);
        timeCounter = 0;
    }
    void animate(const double &frameTime){ //animate coin
        timeCounter += frameTime;
        if(timeCounter >= (double)1/FPS){
            if(actualFrame == 5){
                actualFrame = 0;
            }else{
                actualFrame++;
            }
            timeCounter = 0;
        }
        setTextureRect(frames[actualFrame]);
    }
    bool getCoin(const sf::Sprite &sprite){ //check if guy touched coin
        sf::FloatRect guy = getGlobalBounds();
        sf::FloatRect coin = sprite.getGlobalBounds();
        if( ( (guy.top < coin.top + coin.height && guy.top > coin.top) ||
                        (guy.top + guy.height > coin.top && guy.top < coin.top) ) &&
                                    ( (guy.left < coin.left + coin.width && guy.left > coin.left) ||
                                                (guy.left + guy.width > coin.left && guy.left < coin.left) ) ){
            return true;
        }else{
            return false;
        }
    }
};

class Score: public sf::Text{ //score textbox
private:
    sf::Font font;
public:
    Score(){
        font.loadFromFile("fonts/ARLRDBD.ttf");
        setFont(font);
        setCharacterSize(40 * scaleDesktop.y);
    }
    void updateScore(const std::string &text){
        setString(text);
        sf::FloatRect bounds = getLocalBounds();
        setOrigin((bounds.left + bounds.width)/2, 0);
        setPosition(desktop.width/2, 0);
    }
};

class Guy: public sf::Sprite{ //guy character
private:
    std::vector<sf::IntRect> frames;
    sf::Texture texture;
    double runSpeed, jumpSpeed, speedX, speedY, gravityVal;
    double platformSpeedX, platformSpeedY;
    int FPS = 7; //FPS of animation
    size_t actualFrame;
    double timeCounter;
    sf::Vector2f spawnCoordinates;
    bool respawn;
public:
    bool fallDown;
    Guy(){
        texture.loadFromFile("res/guy.png");
        for(int i = 0; i <= 700; i += 50){ //load all animation frames
            frames.emplace_back(sf::IntRect(i+14, 7, 22, 29));
        }
        actualFrame  = 0;
        setTextureRect(sf::IntRect(frames[actualFrame]));
        setTexture(texture);
        scale(3*scaleDesktop.x, 3*scaleDesktop.y);
        spawnCoordinates = sf::Vector2f((100 * scaleDesktop.x - 300), desktop.height - 5 - ((162 * scaleDesktop.y) + getGlobalBounds().height)); //162 is 220 - 58, it's an y coordinate of grass on ground textures
        setPosition(spawnCoordinates);
        runSpeed = 250 * scaleDesktop.x;
        jumpSpeed = 650 * scaleDesktop.y;
        gravityVal = 1000 * scaleDesktop.y;
        respawn = true;
        fallDown = false;
        platformSpeedX = 0;
        platformSpeedY = 0;
    }
    void moveGuy(const std::string& direction){ //manage speed of guy
        if(!respawn){
            if(direction == "left"){
                speedX = -runSpeed;
            }else if(direction == "right"){
                speedX = runSpeed;
            }else if(direction == "stop"){
                speedX = 0;
            }
            if(direction == "jump" && !fallDown){
                speedY = -jumpSpeed;
            }
        }
    }
    bool animate(const double &frameTime){
        sf::FloatRect bounds = getGlobalBounds();
        if((bounds.left <= 0 && speedX < 0) || (bounds.left + bounds.width >= desktop.width && speedX > 0)){ //check if guy hit screen border
            speedX = 0;
        }
        if(bounds.top <= 0 && speedY < 0){
            speedY = 0;
        }
        if(bounds.top - 100 >= desktop.height && speedY > 0){ //check if guy hit bottom screen border
            respawn = true;
            setPosition(spawnCoordinates);
            return true;
        }
        if(respawn){ //respawn guy and run from left side of map
            if(getGlobalBounds().left < spawnCoordinates.x + 300){
                speedX = runSpeed;
            }else{
                respawn = false;
                speedX = 0;
            }
        }
        if(!respawn && fallDown){
            speedY += (gravityVal * frameTime); //gravity
        }else{
            if(speedY > 0){
                speedY = 0;
            }
        }
        move((speedX + platformSpeedX) * frameTime, (speedY + platformSpeedY) * frameTime);
        timeCounter += frameTime; //guy animation
        if(timeCounter >= (double)1/FPS){
            if(speedX == 0){
                if(actualFrame >= 2){
                    actualFrame = 0;
                }else{
                    actualFrame++;
                }
            }else{
                if(speedX >= 0){
                    if(actualFrame >= 7 || actualFrame < 4){
                        actualFrame = 4;
                    }else{
                        actualFrame++;
                    }
                }else{
                    if(actualFrame >= 13 || actualFrame < 9){
                        actualFrame = 9;
                    }else{
                        actualFrame++;
                    }
                }
            }
            timeCounter = 0;
        }
        setTextureRect(frames[actualFrame]);
        return false;
    }
    bool onGround(sf::Sprite &sprite){ //return true if guy is standing on sprite
        sf::FloatRect guy = getGlobalBounds();
        guy.left += 18; //crop guy texture border
        guy.width -= 33;
        sf::FloatRect floor = sprite.getGlobalBounds();
        if((typeid(sprite)) == (typeid(Ground))){
            floor.top += 60 * sprite.getScale().y; //crop grass from ground border
        }else{
            floor.top += 10 * sprite.getScale().y; //crop grass from platfrom borders
            floor.left += 8 * sprite.getScale().x;
            floor.left -= 14 * sprite.getScale().x;
        }
        if((guy.left + guy.width > floor.left && guy.left < floor.left + floor.width) && (guy.top + guy.height >= floor.top - 4 && guy.top + guy.height <= floor.top + 4)){
            MovingPlatform *movingPlatformPtr = dynamic_cast<MovingPlatform *>(&sprite);
            if(movingPlatformPtr != nullptr){ //check if guy is standing on moving platform
                platformSpeedX = movingPlatformPtr->speedX;
                platformSpeedY = movingPlatformPtr->speedY;
            }else{
                platformSpeedX = 0;
                platformSpeedY = 0;
            }
            return true;
        }else{
            return false;
            platformSpeedX = 0 ;
            platformSpeedY = 0;
        }
    }
};
