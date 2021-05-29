#include "platformer_objects.cpp"
#include <SFML/Window.hpp>

void resetGame(std::vector<std::unique_ptr<Coin>> &coins){
    coins.clear();
    //create coin object. Coin(position_x, positiotn_y)
    coins.emplace_back(std::make_unique<Coin>(50, 150));
    coins.emplace_back(std::make_unique<Coin>(1800, 100));
    coins.emplace_back(std::make_unique<Coin>(1650, 818));
    coins.emplace_back(std::make_unique<Coin>(850, 970));
}

int main(){
    sf::RenderWindow window(desktop, "Platformer", sf::Style::None);
    double FPS = 200; //target maximum number of frames per second
    Background background;
    //create ground at the bottom of screen. Ground(from_x_coordinate, to_x_coordinate)
    std::vector<std::unique_ptr<Ground>> ground;
    ground.emplace_back(std::make_unique<Ground>(0, 800));
    ground.emplace_back(std::make_unique<Ground>(1600, 1750));
    std::vector<std::unique_ptr<Platform>> platforms;
    //create static platform. Platform(position_x, positiotn_y, scale_width). For scale_width = 1 platform is 355px wide.
    platforms.emplace_back(std::make_unique<Platform>(100, 700, 0.5));
    platforms.emplace_back(std::make_unique<Platform>(300, 550, 0.5));
    platforms.emplace_back(std::make_unique<Platform>(490, 200, 0.125));
    platforms.emplace_back(std::make_unique<Platform>(-7, 250, 0.8));
    platforms.emplace_back(std::make_unique<Platform>(1750, 200, 0.5));
    platforms.emplace_back(std::make_unique<Platform>(794, 918, 0.6));
    platforms.emplace_back(std::make_unique<Platform>(794, 1050, 0.5));
    //create moving platform. MovingPlatform(start_position_x, start_positiotn_y, scale_width, move_from_x, move_from_y, move_to_x, move_to_y, speed_on_x, speed_on_y).
    //For scale_width = 1 platform is 355px wide. This will work properly under conditions: move_from_x <= start_position_x <= move_to_x, and the samy for 'y'.
    platforms.emplace_back(std::make_unique<MovingPlatform>(750, 350, 0.75, 750, 250, 700, 450, 0, 100));
    platforms.emplace_back(std::make_unique<MovingPlatform>(1250, 250, 0.5, 1150, 250, 1350, 250, -100, 0));
    platforms.emplace_back(std::make_unique<MovingPlatform>(1700, 600, 0.5, 1700, 450, 1700, 750, 0, 30));
    platforms.emplace_back(std::make_unique<MovingPlatform>(1050, 690, 0.5, 1050, 690, 1300, 690, -70, 0));
    std::vector<std::unique_ptr<Coin>> coins;
    resetGame(coins);
    Guy guy;
    Score score;
    sf::Clock clock;
    double frameTime = 0;
    while (window.isOpen()) {
        sf::Time elapsed = clock.restart();
        if(frameTime >= 1.0/FPS){ //statement to reduce FPS to target amount
            sf::Event event;
            //management section
            while (window.pollEvent(event)) {
                if(event.type == sf::Event::Closed || (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)){
                    window.close();
                }
                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Space) {
                    guy.moveGuy("jump");
                }
            }
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
                guy.moveGuy("left");
            }else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
                guy.moveGuy("right");
            }else{
                guy.moveGuy("stop");
            }
            bool isGuyOnPlatform = std::any_of(platforms.begin(), platforms.end(), [&guy](auto &platform){ return guy.onGround(*platform); });
            bool isGuyOnGround = std::any_of(ground.begin(), ground.end(), [&guy](auto &g){ return guy.onGround(*g); });
            if(!(isGuyOnPlatform || isGuyOnGround)){ //check if guy is standing on sth
                guy.fallDown = true;
            }else{
                guy.fallDown = false;
            }
            //animations section
            if(guy.animate(frameTime)){ //animate guy, it returns true if guy fall down from scene
                resetGame(coins);
            }
            for(auto &platform: platforms){ //animate moving platforms
                MovingPlatform *movingPlatform = dynamic_cast<MovingPlatform *>(platform.get());
                if(movingPlatform != nullptr){
                    movingPlatform->movePlatform(frameTime);
                }
            }
            for(auto it = coins.begin(); it < coins.end(); it++){ //animate coins
                (*it)->animate(frameTime);
                if((*it)->getCoin(guy)){
                    coins.erase(it); //delete coin if guy touch it
                    it--;
                }
            }
            if(coins.size()){
                score.updateScore("Collected coins: " + std::to_string(4 - coins.size()));
            }else{
                score.updateScore("Congratulations! You've got all coins");
            }
            //draw section
            window.clear(sf::Color::Black);
            window.draw(background);
            for(auto &g: ground){
                window.draw(*g);
            }
            for(auto &platform: platforms){
                window.draw(*platform);
            }
            for(auto &coin: coins){
                window.draw(*coin);
            }
            window.draw(guy);
            window.draw(score);
            window.display();
            frameTime = 0;
        }else{
            frameTime += elapsed.asMicroseconds() / 1.e6;
        }
    }
    return 0;
}
