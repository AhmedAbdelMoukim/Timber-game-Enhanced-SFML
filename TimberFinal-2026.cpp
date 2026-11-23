// Timber.cpp - Version compatible avec SFML 3.1.0
// Compiler en C++17 ou plus recent.

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
//#include <SFML/Window.hpp>


#include <ctime>    //Working with dates and time (Getting the current date and time when saving a score.)
#include <fstream>  //Reading and writing files (Saving player scores in scores.txt)
#include <iomanip>  //Formatting output (Using std::put_time() to format the date and time)
#include <iostream> //Input and output in the console (std::cout and std::cerr to display messages and errors.)
#include <random>   //Generating random numbers (Randomizing the bee's position, cloud speeds and branch positions.)
#include <sstream>  //Creating strings using stream operations (Building formatted text, such as displaying the current score.)
#include <string>   //Working with text strings (Storing the player's name using std::string.)


// -----------------------------------------------------------------------------
// Branches
// -----------------------------------------------------------------------------
constexpr std::size_t NUM_BRANCHES = 6; //6 branches qui bougent sur notre arbre
enum class Side { LEFT, RIGHT, NONE };//énumération pour déterminer où est le joueur
std::array<Side, NUM_BRANCHES> branchPositions{}; //Initialiser notre tableau

//Helper method to create a random number generator called randomEngine using - Mersenne Twister 19937.
void updateBranches(std::mt19937& randomEngine)
{
    // Déplacez toutes les branches vers le bas d'une position:  
    // Loop backwards because we want to copy each previous position into the next position without overwriting information that we still need
	for (std::size_t i = NUM_BRANCHES - 1; i > 0; --i)//start from the last index and move down to 1 (not 0 because we don't want to overwrite the first position)
        branchPositions[i] = branchPositions[i - 1];


    //This creates a distribution that generates an integer between 0 and 4, inclusive.
    std::uniform_int_distribution<int> distribution(0, 4);
    const int result = distribution(randomEngine);

    // 0 = branche a gauche, 1 = branche a droite, 2-4 = aucune branche.
    if (result == 0)
        branchPositions[0] = Side::LEFT;
    else if (result == 1)
        branchPositions[0] = Side::RIGHT;
    else
        branchPositions[0] = Side::NONE;
}

//Helper method to center text objects - receives the original text object by reference - modify without creating a copy
void centerText(sf::Text& text, float x, float y)  
{
	const sf::FloatRect bounds = text.getLocalBounds();         //Get the dimensions of the text (left, top, width, height) in local coordinates
    text.setOrigin({ bounds.position.x + bounds.size.x / 2.f,
                    bounds.position.y + bounds.size.y / 2.f }); //By default, SFML positions text using its top-left origin.
                                                                //setOrigin() changes that reference point to the center of the text.
    text.setPosition({ x, y });
}

//Helper method to load an image file into an SFML texture (arguments are by reference)
bool loadTexture(sf::Texture& texture, const std::string& filename) 
{
    if (!texture.loadFromFile(filename))
    {
        std::cerr << "Impossible de charger la texture : " << filename << '\n';
        return false;
    }
    return true;
}

//Helper method to write the player's score and name to a text file called scores.txt
void writeScores(int playerScore, const std::string& playerName)
{
    const std::time_t now = std::time(nullptr);
    std::tm localTime{};

    const bool timeValid = (localtime_s(&localTime, &now) == 0);

    std::ofstream outputFile("scores.txt", std::ios::app);
    if (!outputFile)
    {
        std::cerr << "Impossible d'ouvrir scores.txt pour l'ecriture.\n";
        return;
    }

    outputFile << "Timber Scores on : ";
    if (timeValid)
        outputFile << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
    else
        outputFile << "date inconnue";

    outputFile << "\nPlayer Name : " << playerName
        << "\tScore: " << playerScore << "\n\n";

    std::cout << "Score sauvegarde.\n";
}


// C'est ici que commence notre jeu, à partir de int main()
int main()
{
    // -------------------------------------------------------------------------
    // Fenêtre plein écran - d'après la résolution de l'écran
    // -------------------------------------------------------------------------
    const sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();
    const unsigned int vw = desktopMode.size.x;
    const unsigned int vh = desktopMode.size.y;

    sf::RenderWindow window(
        sf::VideoMode({ vw, vh }),
        "Timber!!!",
        sf::Style::Default,
        sf::State::Fullscreen
    );

    //setting optimization options
    window.setVerticalSyncEnabled(true); //limit the number of frames displayed to the refresh rate of the monitor, for example 60Hz
    window.setKeyRepeatEnabled(false); //prevent from receiving repeated KeyPressed events while keeping a key pressed


    // Générateur aléatoire pour l'abeille et les nuages: initialisé une seule fois.
    std::random_device randomDevice;
    std::mt19937 randomEngine(randomDevice());
    std::uniform_int_distribution<int> beeSpeedDistribution(200, 399);      //Bee's horizontal speed between 200 and 399 -  fast
    std::uniform_int_distribution<int> beeHeightDistribution(500, 999);     //Bee's vertical position between 500 and 999
    std::uniform_int_distribution<int> cloudSpeedDistribution(0, 199);      //Cloud's horizontal speed between 0 and 199 - slow
    std::uniform_int_distribution<int> cloud1HeightDistribution(0, 149);    //Vertical position of cloud 1
    std::uniform_int_distribution<int> cloud2HeightDistribution(-150, 149); //Vertical position of cloud 2
    std::uniform_int_distribution<int> cloud3HeightDistribution(-150, 299); //Vertical position of cloud 3

    // -------------------------------------------------------------------------
    // Textures et sprites d'arriere-plan
    // -------------------------------------------------------------------------
    sf::Texture textureBackground;
    sf::Texture textureTree;
    sf::Texture arbreTexture;
    sf::Texture textureBee;
    sf::Texture textureCloud;
    sf::Texture textureBranch;
    sf::Texture texturePlayer;
    sf::Texture textureRIP;
    sf::Texture textureAxe;
    sf::Texture textureLog;

    if (!loadTexture(textureBackground, "graphics/background.png") ||
        !loadTexture(textureTree, "graphics/tree.png") ||
        !loadTexture(arbreTexture, "Graphics/arbre2.png") ||
        !loadTexture(textureBee, "graphics/bee.png") ||
        !loadTexture(textureCloud, "graphics/cloud.png") ||
        !loadTexture(textureBranch, "graphics/branch.png") ||
        !loadTexture(texturePlayer, "graphics/player.png") ||
        !loadTexture(textureRIP, "graphics/rip.png") ||
        !loadTexture(textureAxe, "graphics/axe.png") ||
        !loadTexture(textureLog, "graphics/log.png"))
    {
        return -1;
    }

    sf::Sprite spriteBackground(textureBackground);
    spriteBackground.setPosition({ 0.f, 0.f });

    sf::Sprite spriteTree(textureTree);
    spriteTree.setPosition({ 810.f, 0.f });

    std::vector<sf::Sprite> trees;
    trees.reserve(3);
    for (int i = 0; i < 3; ++i)
    {
        sf::Sprite tree(arbreTexture);
        tree.setPosition({ static_cast<float>(i * 150), 150.f + i * 50.f });
        trees.push_back(tree);
    }

    // -------------------------------------------------------------------------
    // Abeille : préparer un sprite pour l'abeille
    // -------------------------------------------------------------------------
    sf::Sprite spriteBee(textureBee);
    spriteBee.setPosition({ 0.f, 500.f });
    bool beeActive = false;  // L'abeille bouge-t-elle actuellement ?
    float beeSpeed = 0.f;    // À quelle vitesse l'abeille peut-elle voler

    // -------------------------------------------------------------------------
    // Nuages : préparer 3 sprites de nuages ​​à partir d'une seule texture
    // -------------------------------------------------------------------------
    sf::Sprite spriteCloud1(textureCloud);
    sf::Sprite spriteCloud2(textureCloud);
    sf::Sprite spriteCloud3(textureCloud);

    // Positionnez les nuages ​​à gauche de l'écran à différentes hauteurs (Y)
    spriteCloud1.setPosition({ 0.f, 0.f });
    spriteCloud2.setPosition({ 0.f, 250.f });
    spriteCloud3.setPosition({ 0.f, 500.f });

    // Les nuages ​​sont-ils actuellement à l'écran ?
    bool cloud1Active = false;
    bool cloud2Active = false;
    bool cloud3Active = false;
  
    // Quelle est la vitesse de chaque cloud ?
    float cloud1Speed = 0.f;
    float cloud2Speed = 0.f;
    float cloud3Speed = 0.f;

    // -------------------------------------------------------------------------
    // Police et textes (SFML 3 exige la police dans le constructeur de Text)
    // -------------------------------------------------------------------------
    sf::Font font;
    if (!font.openFromFile("fonts/KOMIKAP_.ttf"))
    {
        std::cerr << "Impossible de charger fonts/KOMIKAP_.ttf\n";
        return -1;
    }

    sf::Text messageText(font);
    sf::Text scoreText(font);
    messageText.setString("Press Enter to start!");
    scoreText.setString("Score = 0");
    messageText.setCharacterSize(75);
    scoreText.setCharacterSize(100);
    messageText.setFillColor(sf::Color::White);
    scoreText.setFillColor(sf::Color::White);
    centerText(messageText, vw / 2.f, vh / 2.f);
    scoreText.setPosition({ 20.f, 20.f });

    // Saisie du nom du joueur
    sf::Text instructions(font, "Enter your name: ", 20);
    instructions.setFillColor(sf::Color::Yellow);
    instructions.setPosition({ 300.f, 300.f });

    sf::Text inputText(font, "", 20);
    inputText.setFillColor(sf::Color::Red);
    inputText.setPosition({ 500.f, 300.f });
    std::string playerName;

    // -------------------------------------------------------------------------
    // Barre de temps et variables de jeu pour l'animation
    // -------------------------------------------------------------------------
    //sf::RectangleShape timeBar({ 400.f, 80.f });
    //constexpr float timeBarStartWidth = 400.f;
    //constexpr float timeBarHeight = 80.f;
    constexpr float timeBarStartWidth = 200.f;
    //constexpr float timeBarHeight = 80.f;
    constexpr float timeBarHeight = 40.f;
    sf::RectangleShape timeBar({ timeBarStartWidth, timeBarHeight });
    timeBar.setFillColor(sf::Color::Red);

    // set a 5-pixel wide orange outline
    timeBar.setOutlineThickness(5.f);
    timeBar.setOutlineColor(sf::Color(250, 150, 100));

    timeBar.setPosition({ 5.f, static_cast<float>(vh) - timeBarHeight });// 5 pixels du bord gauche 

    float timeRemaining = 6.f;
    float timeBarWidthPerSecond = timeBarStartWidth / timeRemaining;
    int score = 0;
    bool paused = true;
    bool acceptInput = false;
    sf::Clock clock;

    // -------------------------------------------------------------------------
    // Branches
    // -------------------------------------------------------------------------
    std::array<sf::Sprite, NUM_BRANCHES> branches = {
        sf::Sprite(textureBranch), sf::Sprite(textureBranch),
        sf::Sprite(textureBranch), sf::Sprite(textureBranch),
        sf::Sprite(textureBranch), sf::Sprite(textureBranch)
    };

    for (std::size_t i = 0; i < NUM_BRANCHES; ++i)
    {
        branches[i].setPosition({ -2000.f, -2000.f });
        branches[i].setOrigin({ 220.f, 20.f });
        branchPositions[i] = Side::NONE;
    }

    // -------------------------------------------------------------------------
    // Joueur, pierre tombale, hache et buche
    // -------------------------------------------------------------------------
    sf::Sprite spritePlayer(texturePlayer);
    spritePlayer.setPosition({ 580.f, 580.f });
    Side playerSide = Side::LEFT;

    sf::Sprite spriteRIP(textureRIP);
    spriteRIP.setPosition({ 400.f, 620.f });

    sf::Sprite spriteAxe(textureAxe);
    spriteAxe.setPosition({ 700.f, 710.f });
    constexpr float AXE_POSITION_LEFT = 700.f;
    constexpr float AXE_POSITION_RIGHT = 1075.f;

    sf::Sprite spriteLog(textureLog);
    spriteLog.setPosition({ 810.f, 720.f });
    bool logActive = false;
    float logSpeedX = 1000.f;
    float logSpeedY = -1500.f;

    // -------------------------------------------------------------------------
    // Sons : SFML 3 construit Sound avec le SoundBuffer correspondant.
    // Les buffers doivent rester en vie aussi longtemps que les sons.
    // -------------------------------------------------------------------------
    sf::SoundBuffer chopBuffer;
    sf::SoundBuffer deathBuffer;
    sf::SoundBuffer ootBuffer;

    if (!chopBuffer.loadFromFile("sound/chop.wav") ||
        !deathBuffer.loadFromFile("sound/death.wav") ||
        !ootBuffer.loadFromFile("sound/out_of_time.wav"))
    {
        std::cerr << "Impossible de charger un ou plusieurs fichiers sonores.\n";
        return -1;
    }

    sf::Sound chop(chopBuffer);
    sf::Sound death(deathBuffer);
    sf::Sound outOfTime(ootBuffer);

    // -------------------------------------------------------------------------
    // Boucle principale - GAME LOOP
    // -------------------------------------------------------------------------
    while (window.isOpen())
    {
        // Evénements SFML 3 : pollEvent() retourne un optional<Event>.
        while (const std::optional<sf::Event> event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            // Saisie du nom (caractères ASCII imprimables).
            if (const auto* textEvent = event->getIf<sf::Event::TextEntered>())
            {
                if (textEvent->unicode == U'\b' && !playerName.empty())
                {
                    playerName.pop_back();
                }
                else if (textEvent->unicode >= 32 && textEvent->unicode <= 126)
                {
                    playerName += static_cast<char>(textEvent->unicode);
                }
                inputText.setString(playerName);
            }

            // Une touche relâchée permet de recevoir la prochaine action.
            if (event->is<sf::Event::KeyReleased>() && !paused)
            {
                acceptInput = true;
                spriteAxe.setPosition({ 2000.f, spriteAxe.getPosition().y });
            }

            // Démarrer/réinitialiser avec Entrée (une seule fois par pression).
            if (const auto* keyEvent = event->getIf<sf::Event::KeyPressed>())
            {
                if (keyEvent->code == sf::Keyboard::Key::Escape)
                    window.close();

                if (keyEvent->code == sf::Keyboard::Key::Enter)
                {
                    paused = false;
                    score = 0;
                    timeRemaining = 6.f;
                    timeBarWidthPerSecond = timeBarStartWidth / timeRemaining;
                    timeBar.setSize({ timeBarStartWidth, timeBarHeight });
                    messageText.setString("Press Enter to start!");
                    centerText(messageText, vw / 2.f, vh / 2.f);
                    instructions.setPosition({ 2000.f, 660.f });

                    for (std::size_t i = 0; i < NUM_BRANCHES; ++i)
                        branchPositions[i] = Side::NONE;

                    spriteRIP.setPosition({ 675.f, 2000.f });
                    spritePlayer.setPosition({ 580.f, 580.f });
                    spriteAxe.setPosition({ 2000.f, spriteAxe.getPosition().y });
                    spriteLog.setPosition({ 810.f, 580.f });
                    logActive = false;
                    playerSide = Side::LEFT;
                    acceptInput = true;
                    clock.restart();
                }

                if (keyEvent->code == sf::Keyboard::Key::Space)
                    paused = true;
            }
		}// End of event polling

        if (!window.isOpen())
            break;

        // ---------------------------------------------------------------------
        // Entrées du joueur
        // ---------------------------------------------------------------------
        if (acceptInput && !paused)
        {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
            {
                playerSide = Side::RIGHT;
                ++score;
                timeRemaining += (2.f / score) + 0.15f;
                spriteAxe.setPosition({ AXE_POSITION_RIGHT, spriteAxe.getPosition().y });
                spritePlayer.setPosition({ 1200.f, 580.f });
                updateBranches(randomEngine);
                spriteLog.setPosition({ 810.f, 580.f });
                logSpeedX = -5000.f;
                logActive = true;
                acceptInput = false;
                chop.play();
            }

            if (acceptInput && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
            {
                playerSide = Side::LEFT;
                ++score;
                timeRemaining += (2.f / score) + 0.15f;
                spriteAxe.setPosition({ AXE_POSITION_LEFT, spriteAxe.getPosition().y });
                spritePlayer.setPosition({ 590.f, 580.f });
                updateBranches(randomEngine);
                spriteLog.setPosition({ 810.f, 580.f });
                logSpeedX = 5000.f;
                logActive = true;
                acceptInput = false;
                chop.play();
            }
        }

        // ---------------------------------------------------------------------
        // Mise a jour de la scene
        // ---------------------------------------------------------------------
        if (!paused)
        {
            const sf::Time dt = clock.restart();
            const float deltaTime = dt.asSeconds();

            timeRemaining -= deltaTime;
            timeBar.setSize({ std::max(0.f, timeBarWidthPerSecond * timeRemaining), timeBarHeight });

            if (timeRemaining <= 0.f)
            {
                timeRemaining = 0.f;
                paused = true;
                acceptInput = false;
                messageText.setString("Out of time!!");
                centerText(messageText, vw / 2.f, vh / 2.f);
                outOfTime.play();
            }

            // Abeille
            if (!beeActive)
            {
                beeSpeed = static_cast<float>(beeSpeedDistribution(randomEngine));
                const float height = static_cast<float>(beeHeightDistribution(randomEngine));
                spriteBee.setPosition({ 2000.f, height });
                beeActive = true;
            }
            else
            {
                spriteBee.setPosition({ spriteBee.getPosition().x - beeSpeed * deltaTime,
                                       spriteBee.getPosition().y });
                if (spriteBee.getPosition().x < -100.f)
                    beeActive = false;
            }

            // Nuage 1
            if (!cloud1Active)
            {
                cloud1Speed = static_cast<float>(cloudSpeedDistribution(randomEngine));
                const float height = static_cast<float>(cloud1HeightDistribution(randomEngine));
                spriteCloud1.setPosition({ -200.f, height });
                cloud1Active = true;
            }
            else
            {
                spriteCloud1.setPosition({ spriteCloud1.getPosition().x + cloud1Speed * deltaTime,
                                          spriteCloud1.getPosition().y });
                if (spriteCloud1.getPosition().x > static_cast<float>(vw))
                    cloud1Active = false;
            }

            // Nuage 2
            if (!cloud2Active)
            {
                cloud2Speed = static_cast<float>(cloudSpeedDistribution(randomEngine));
                const float height = static_cast<float>(cloud2HeightDistribution(randomEngine));
                spriteCloud2.setPosition({ -200.f, height });
                cloud2Active = true;
            }
            else
            {
                spriteCloud2.setPosition({ spriteCloud2.getPosition().x + cloud2Speed * deltaTime,
                                          spriteCloud2.getPosition().y });
                if (spriteCloud2.getPosition().x > static_cast<float>(vw))
                    cloud2Active = false;
            }

            // Nuage 3
            if (!cloud3Active)
            {
                cloud3Speed = static_cast<float>(cloudSpeedDistribution(randomEngine));
                const float height = static_cast<float>(cloud3HeightDistribution(randomEngine));
                spriteCloud3.setPosition({ -200.f, height });
                cloud3Active = true;
            }
            else
            {
                spriteCloud3.setPosition({ spriteCloud3.getPosition().x + cloud3Speed * deltaTime,
                                          spriteCloud3.getPosition().y });
                if (spriteCloud3.getPosition().x > static_cast<float>(vw))
                    cloud3Active = false;
            }

            // Mise a jour du score
            std::ostringstream scoreStream;
            scoreStream << "Score = " << score;
            scoreText.setString(scoreStream.str());

            // Position et orientation des branches
            for (std::size_t i = 0; i < NUM_BRANCHES; ++i)
            {
                const float height = static_cast<float>(i * 150);
                if (branchPositions[i] == Side::LEFT)
                {
                    branches[i].setPosition({ 610.f, height });
                    branches[i].setRotation(sf::degrees(180.f));
                }
                else if (branchPositions[i] == Side::RIGHT)
                {
                    branches[i].setPosition({ 1330.f, height });
                    branches[i].setRotation(sf::degrees(0.f));
                }
                else
                {
                    branches[i].setPosition({ 3000.f, height });
                }
            }

            // Animation de la buche
            if (logActive)
            {
                spriteLog.setPosition({ spriteLog.getPosition().x + logSpeedX * deltaTime,
                                       spriteLog.getPosition().y + logSpeedY * deltaTime });
                if (spriteLog.getPosition().x < -100.f || spriteLog.getPosition().x > 2000.f)
                {
                    logActive = false;
                    spriteLog.setPosition({ 810.f, 720.f });
                }
            }

            // Collision avec une branche
            if (branchPositions[5] == playerSide)
            {
                paused = true;
                acceptInput = false;
                spriteRIP.setPosition({ 200.f, 510.f });
                spritePlayer.setPosition({ 2000.f, 660.f });
                spriteAxe.setPosition({ 2000.f, 830.f });
                messageText.setString("SQUISHED!!");
                centerText(messageText, vw / 2.f, vh / 2.f);
                death.play();
                writeScores(score, playerName);
            }
        }

        // ---------------------------------------------------------------------
        // Affichage
        // ---------------------------------------------------------------------
        window.clear();
        window.draw(spriteBackground);

        for (const sf::Sprite& tree : trees)
            window.draw(tree);

        window.draw(instructions);
        window.draw(inputText);
        window.draw(spriteCloud1);
        window.draw(spriteCloud2);
        window.draw(spriteCloud3);
        window.draw(spriteTree);

        for (const sf::Sprite& branch : branches)
            window.draw(branch);

        window.draw(spriteBee);
        window.draw(scoreText);
        window.draw(spritePlayer);
        window.draw(spriteAxe);
        window.draw(spriteLog);
        window.draw(spriteRIP);
        window.draw(timeBar);

        if (paused)
            window.draw(messageText);

        window.display();
	}//End of game loop

    return 0;
}//End of main()
