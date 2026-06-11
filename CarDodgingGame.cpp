//Car dodging Game 
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <filesystem>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <fstream>

using namespace std;

//----------------------------------------*Game states*----------------------------------- 
enum GameState
{
	SPLASH,
	MENU,
	ENTERNAME,
	CARSELECT,
	PLAYING,
	PAUSED,
	GAMEOVER,
	HELP,
	HIGHSCORES,
	OPTIONS
};

//--------------------------------------------*FUNCTIONS*-------------------------------
//non returning and non parametrized function
void SplashDummy()
{
	// just logging splash state
}

//Returning function for BUTTONS
sf::RectangleShape CreateButton(
	float width,
	float height,
	sf::Color color,
	float x,
	float y)
{
	sf::RectangleShape button;

	button.setSize(sf::Vector2f(width, height));

	button.setFillColor(color);

	button.setPosition(x, y);

	return button;
}

//Returning function for TEXT
sf::Text CreateText(
	sf::Font& font,
	string textString,
	int size,
	sf::Color color,
	float x,
	float y)
{
	sf::Text text;

	text.setFont(font);
	text.setString(textString);
	text.setCharacterSize(size);
	text.setFillColor(color);
	text.setPosition(x, y);

	return text;
}

//Reset game nonreturning function
void ResetGame(
	int& score,
	int& lives,
	sf::Sprite& car,
	std::vector<sf::Sprite>& enemies)
{
	score = 0;
	lives = 3;

	car.setPosition(1050, 615);

	enemies[0].setPosition(100, 0);
	enemies[1].setPosition(445, -200);
	enemies[2].setPosition(750, -400);
	enemies[3].setPosition(1050, -600);
	enemies[4].setPosition(1350, -800);
	enemies[5].setPosition(1610, -1000);
	enemies[6].setPosition(750, -2000);
}

//Handling Coollision
void HandleCollision(
	sf::Sprite& car,
	std::vector<sf::Sprite>& enemies,
	int& lives,
	bool& safeMode,
	sf::Clock& safeClock,
	sf::Sprite& spark,
	bool& showSpark,
	sf::Clock& sparkClock,
	sf::Sound& crashSound,
	GameState& currentState)
{
	for (size_t i = 0; i < enemies.size(); i++)
	{
		sf::FloatRect playerBounds = car.getGlobalBounds();
		sf::FloatRect enemyBounds = enemies[i].getGlobalBounds();

		// shrink hitboxes
		playerBounds.left += 9;
		playerBounds.width -= 18;
		playerBounds.top += 10;
		playerBounds.height -= 16;

		enemyBounds.left += 9;
		enemyBounds.width -= 18;
		enemyBounds.top += 10;
		enemyBounds.height -= 16;

		if (currentState == PLAYING &&
			!safeMode &&
			playerBounds.intersects(enemyBounds))
		{
			// spark position
			sf::FloatRect overlap;
			playerBounds.intersects(enemyBounds, overlap);

			float sparkX = overlap.left + overlap.width / 2;
			float sparkY = overlap.top + overlap.height / 2;

			spark.setPosition(sparkX, sparkY);
			showSpark = true;
			sparkClock.restart();

			crashSound.play();

			lives--;

			if (lives <= 0)
			{
				currentState = GAMEOVER;
				safeMode = false;
				car.setColor(sf::Color::White);
			}
			else
			{
				safeMode = true;
				safeClock.restart();
				car.setColor(sf::Color(255, 255, 255, 120));

				// move enemy away
				enemies[i].setPosition(
					enemies[i].getPosition().x,
					-500
				);
			}
		}
	}
}

//Non returning function for Enemy spawning
void RespawnEnemy(sf::Sprite& enemy,
	std::vector<sf::Sprite>& enemies,
	int lanes[])
{
	int newLane;
	int randomY;
	bool positionOK;

	do
	{
		positionOK = true;

		newLane = rand() % 6;

		randomY = -(rand() % 1200 + 400);

		for (int j = 0; j < enemies.size(); j++)
		{
			if (&enemy != &enemies[j])
			{
				if (enemies[j].getPosition().x == lanes[newLane])
				{
					if (abs(enemies[j].getPosition().y - randomY) < 1300)
					{
						positionOK = false;
					}
				}
			}
		}

	} while (!positionOK);

	enemy.setPosition(lanes[newLane], randomY);
}


//----------------------------------------------*MAIN FUNCTION*----------------------------------------------
int main()
{
	//ctime ko use kiya takay random spawning ki position bhi change ho 
	srand(time(0));

	//To load window
	sf::RenderWindow window(sf::VideoMode(1920, 1080),"Car Dodging Game");

	//to call all the events that are occuring on window
	sf::Event event;

	//------------------------------------------*Textures*-----------------------------------------------
	//Texture sprite outside loop to load ad make the sprite of car and set its position draw inside the loop as it runs 100000/sec 
	
	//PLAYER CAR 
	sf::Texture cartexture;
	cartexture.loadFromFile("ASA_Car.png");
	sf::Sprite car(cartexture);
	car.setPosition(1050, 615);
	car.setScale(0.27f, 0.27f);
	
	//Texture sprite Road
	sf::Texture roadtexture;
	roadtexture.loadFromFile("road.png");
	roadtexture.setSmooth(false);

	sf::Sprite road(roadtexture);
	sf::Sprite road2(roadtexture);
	road.setPosition(0, 0);
	road2.setPosition(0, -1065);

	//Texture sprite Spark
	sf::Texture sparkTexture;
	sparkTexture.loadFromFile("Boom.png");
	sf::Sprite spark(sparkTexture);
	spark.setScale(0.15f, 0.15f);

	spark.setOrigin(
		spark.getLocalBounds().width / 2,
		spark.getLocalBounds().height / 2
	);


	//Player car selection
	sf::Texture car1texture;
	car1texture.loadFromFile("ASA_Car.png");

	sf::Texture car2texture;
	car2texture.loadFromFile("ASA_Car2.png");

	sf::Texture car3texture;
	car3texture.loadFromFile("ASA_Car3.png");

	//Adjusting all three cars
	sf::Sprite Car1(car1texture);
	Car1.setScale(0.27f, 0.27f);
	Car1.setPosition(350, 350);

	sf::Sprite Car2(car2texture);
	Car2.setScale(0.47f, 0.47f);
	Car2.setPosition(795, 355);

	sf::Sprite Car3(car3texture);
	Car3.setScale(0.52f, 0.52f);
	Car3.setPosition(1237, 350);

	// ENEMY SETUP
	
	//Creating vector
	std::vector<sf::Sprite> enemies;

	//Texture Enemies
	sf::Texture blackTexture;
	blackTexture.loadFromFile("Black.png");
	sf::Texture blueTexture;
	blueTexture.loadFromFile("Blue.png");
	sf::Texture greenTexture;
	greenTexture.loadFromFile("Green.png");
	sf::Texture yellowTexture;
	yellowTexture.loadFromFile("Yellow.png");
	sf::Texture purpleTexture;
	purpleTexture.loadFromFile("Purple.png");
	sf::Texture orangeTexture;
	orangeTexture.loadFromFile("Orange.png");
	sf::Texture whiteTexture;
	whiteTexture.loadFromFile("White.png");

	//Adjusting size of Enemies
	sf::Sprite enemy1(blackTexture);
	enemy1.setPosition(100, 0);
	enemy1.setScale(0.46f, 0.46f);
	sf::Sprite enemy2(blueTexture);
	enemy2.setPosition(445, -200);
	enemy2.setScale(0.46f, 0.46f);
	sf::Sprite enemy3(greenTexture);
	enemy3.setPosition(750, -400);
	enemy3.setScale(0.46f, 0.46f);
	sf::Sprite enemy4(purpleTexture);
	enemy4.setPosition(1050, -600);
	enemy4.setScale(0.46f, 0.46f);
	sf::Sprite enemy5(yellowTexture);
	enemy5.setPosition(1350, -800);
	enemy5.setScale(0.38f, 0.38f);
	sf::Sprite enemy6(orangeTexture);
	enemy6.setPosition(1610, -1000);
	enemy6.setScale(0.46f, 0.46f);
	sf::Sprite enemy7(whiteTexture);
	enemy7.setPosition(750, -2000);
	enemy7.setScale(0.34f, 0.34f);

	//Adding enemies in vector basically it is somehow a list storing bin like array
	enemies.push_back(enemy1);
	enemies.push_back(enemy2);
	enemies.push_back(enemy3);
	enemies.push_back(enemy4);
	enemies.push_back(enemy5);
	enemies.push_back(enemy6);
	enemies.push_back(enemy7);


	//Movement through delta time
	float speed = 700;
	float enemyspeed = 700;
	sf::Clock deltaClock;


	//Adding font file used for every text
	sf::Font font;
	font.loadFromFile("PressStart2P-Regular.ttf");

	//-----------------------------------------TEXT SEC--------------------------------------
	//Adding RESTART text
	sf::Text Play = CreateText(
		font,"Restart",50,sf::Color::White,790,655);

	//Adding EXIT text on game over screen
	sf::Text Exit = CreateText(
		font,"Exit",50,sf::Color::White,850,825);

	//Adding Gameover text
	sf::Text gameOverText = CreateText(
		font,"GAME OVER",110,sf::Color::Red,495,250);

	//Adding Score Text
	sf::Text scoreText = CreateText(
		font,"",25,sf::Color::Black,65,30);

	//Adding Lives Text
	sf::Text livesText = CreateText(
		font,"", 30, sf::Color::Red, 1600, 30);

	//Adding Play Text
	sf::Text menuPlayText = CreateText(
		font,"PLAY",45,sf::Color::White,870,335);

	//Player car selecion text
	sf::Text selectCarText = CreateText(
		font,"SELECT YOUR CAR",60,sf::Color::White,500,120);


	//Paused game text
	sf::Text pauseText = CreateText(
		font,"PAUSED",100,sf::Color::Yellow,685,300);

	//Resume text
	sf::Text resumeText = CreateText(
		font,"RESUME",50,sf::Color::White,835,500);

	//Highscore Text on menu screen
	sf::Text highscoreText = CreateText(
		font,"HIGH SCORES",35,sf::Color::White,770,455);

	//Help Text
	sf::Text helpText = CreateText(
		font,"HELP",40,sf::Color::White,890,575);

	//Options Text
	sf::Text optionsText = CreateText(
		font,"OPTIONS",40,sf::Color::White,835,695);

	//Exit Text on menu screen
	sf::Text menuExitText = CreateText(
		font,"EXIT",40,sf::Color::White,890,815);


	//Back screen instruction text
	sf::Text backText = CreateText(
		font,"Press TAB to go back",25,sf::Color::White,1150,980);

	//Start text
	sf::Text startText = CreateText(
		font,"START",40,sf::Color::White,860,770);

	//Music ON text
	sf::Text musicOnText = CreateText(
		font,"MUSIC ON",35,sf::Color::White,810,320);

	//Music OFF text
	sf::Text musicOffText = CreateText(
		font,"MUSIC OFF",35,sf::Color::White,810,450);

	//Volume UP
	sf::Text volumeUpText = CreateText(
		font,"VOLUME +",35,sf::Color::White,810,580);

	//Volume DOWN
	sf::Text volumeDownText = CreateText(
		font,"VOLUME -",35, sf::Color::White, 810, 710);

	//Help Screen text
	sf::Text helpScreen = CreateText(
		font,
		"LEFT / RIGHT arrows  ->  move your car\n\n"
		"Avoid all enemy cars on the road\n\n"
		"You have 3 lives total\n\n"
		"Colliding loses 1 life\n\n"
		"Speed increases with your score\n\n"
		"After a hit you get 2 sec safe mode\n\n"
		"Press P to pause game\n\n"
		"Survive as long as you can!\n\n"
		"ENJOYYYYY!!!", 35, sf::Color::White, 320, 220);

	//Game Title
	sf::Text gameTitle = CreateText(
		font,"MIDNIGHT DODGERS",95,sf::Color::Yellow,200,80);

	//Game Title
	sf::Text titleShadow = CreateText(
		font,"MIDNIGHT DODGERS",95,sf::Color::Black,208,88);

	//Final score
	sf::Text finalScoreText = CreateText(
		font,"",40,sf::Color::White,630,440);
	
	//Enter your name text
	sf::Text enterNameText = CreateText(
		font, "ENTER YOUR NAME", 40, sf::Color::White, 655, 400);

	//High score display for gameover screen
	sf::Text highScoreText = CreateText(
		font,"",40,sf::Color::Yellow,645,525);

	//Player name appear during game 
	sf::Text playerText = CreateText(
		font,"",20,sf::Color::White,65,90);

	//Game title on splash screen
	sf::Text splashTitle = CreateText(
		font,"MIDNIGHT DODGERS",90,sf::Color::Yellow,260,250);

	//Description on splash screen
	sf::Text presentedBy = CreateText(
		font,"PRESENTED BY:",70,sf::Color::White,545,420);
	//Names on splash screen
	sf::Text namesText = CreateText(
		font,"SAMAHIR SHABBIR ",45,sf::Color::Cyan,610,560);
	//Instructions on splash screen
	sf::Text continueText = CreateText(
		font,"    PRESS ANY KEY TO CONTINUE",30,sf::Color::Green,800,890);



	//-------------------------------------------*Buttons*-----------------------------------

	//Adding RESTART button
	sf::RectangleShape playButton =
		CreateButton(400,80,sf::Color::Red,760,640);

	//Adding EXIT button on gameover screen
	sf::RectangleShape exitButton =
		CreateButton(250,80,sf::Color::Red,820,800);

	// PLAY BUTTON
	sf::RectangleShape menuPlayButton =
		CreateButton(400,80,sf::Color::Blue,760,320);

	//Resume Button
	sf::RectangleShape resumeButton =
		CreateButton(450,90,sf::Color::Blue,760,470);

	//Highscore Button
	sf::RectangleShape highscoreButton =
		CreateButton(400,80,sf::Color::Blue,760,440);

	//Help Button
	sf::RectangleShape helpButton =
		CreateButton(400,80,sf::Color::Blue,760,560);

	//Options Button
	sf::RectangleShape optionsButton =
		CreateButton(400,80,sf::Color::Blue,760,680);

	//Exit Button on menu screen
	sf::RectangleShape menuExitButton =
		CreateButton(400,80,sf::Color::Red, 760, 800);

	//Start button
	sf::RectangleShape startButton =
		CreateButton(250,80,sf::Color::Red,835,750);

	//Music on button
	sf::RectangleShape musicOnButton =
		CreateButton(400,80,sf::Color::Green,750,300);

	//Music off button
	sf::RectangleShape musicOffButton =
		CreateButton(400,80,sf::Color::Red,750,430);

	//Volume Up button
	sf::RectangleShape volumeUpButton =
		CreateButton(400,80,sf::Color::Blue,750,560);

	//Volume down button
	sf::RectangleShape volumeDownButton =
		CreateButton(400,80,sf::Color::Blue,750,690);
//-------------------------------------------------*ADDITIONAL ITEMS*------------------------------------------------

	//dark background 
	sf::RectangleShape overlay;
	overlay.setSize(sf::Vector2f(1920, 1080));
	overlay.setFillColor(sf::Color(0, 0, 0, 200));
	
	//BOX FOR ENTER NAME INPUT
	sf::RectangleShape nameBox;
	nameBox.setSize(sf::Vector2f(600, 70));
	nameBox.setFillColor(sf::Color(80, 80, 80));
	nameBox.setOutlineThickness(3);
	nameBox.setOutlineColor(sf::Color::Yellow);
	nameBox.setPosition(655, 550);

	//Car selection rectangles
	sf::RectangleShape border1;
	border1.setSize(sf::Vector2f(220, 420));
	border1.setOutlineThickness(5);
	border1.setOutlineColor(sf::Color::Red);
	border1.setFillColor(sf::Color::Transparent);
	border1.setPosition(300, 300);

	sf::RectangleShape border2;
	border2.setSize(sf::Vector2f(220, 420));
	border2.setOutlineThickness(5);
	border2.setOutlineColor(sf::Color::Blue);
	border2.setFillColor(sf::Color::Transparent);
	border2.setPosition(750, 300);

	sf::RectangleShape border3;
	border3.setSize(sf::Vector2f(220, 420));
	border3.setOutlineThickness(5);
	border3.setOutlineColor(sf::Color::Green);
	border3.setFillColor(sf::Color::Transparent);
	border3.setPosition(1200, 300);


	// BUTTON SOUND
	sf::SoundBuffer clickBuffer;
	clickBuffer.loadFromFile("click.wav");

	sf::Sound clickSound;
	clickSound.setBuffer(clickBuffer);


	// CRASH SOUND
	sf::SoundBuffer crashBuffer;
	crashBuffer.loadFromFile("crash.wav");

	sf::Sound crashSound;
	crashSound.setBuffer(crashBuffer);


	// BACKGROUND MUSIC
	sf::Music bgMusic;

	if (bgMusic.openFromFile("back.wav"))
	{
		bgMusic.setLoop(true);
		bgMusic.setVolume(40);
		bgMusic.play();
	}
	
	//-------------------------------------------------VARIABLES-----------------------------
	//Array to tell the cordinates of lanes for random spawning
	int lanes[6] = { 100,445,750,1050,1350,1610 };
	//Current State
	GameState currentState = SPLASH;
	//Score sys
	int score = 0;
	//High Score sys
	int highScore = 0;
	//lives sys
	int lives = 3;
	//Safe mode
	bool safeMode = false;
	sf::Clock safeClock;
	//Spark
	bool showSpark = false;
	sf::Clock sparkClock;
	//Player Name
	string playerName = "";

	//-----------------File handling ------------------
	// Load high score from file
	ifstream readFile("highscore.txt");

	if (readFile.is_open())
	{
		readFile >> highScore;
	}

	readFile.close();



	//---------------------------------------------GAME LOOP-------------------------------
	while (window.isOpen())
	{
		//delta time used in whole project to manage fps
		float dt = deltaClock.restart().asSeconds();


		//ROAD MOVEMENT
		if (currentState == PLAYING)
		{
			road.move(0, 400 * dt);
			road2.move(0, 400 * dt);
		}
		//Reseting road position
		if (road.getPosition().y >= 1065)
		{
			road.setPosition(0, -1065);
		}

		if (road2.getPosition().y >= 1065)
		{
			road2.setPosition(0, -1065);
		}

		//SAFE MODE TIMER
		if (safeMode)
		{
			float safeTime = safeClock.getElapsedTime().asSeconds();

			// Car stays transparent during safe mode
			car.setColor(sf::Color(255, 255, 255, 120));

			// End safe mode after 2 seconds
			if (safeTime > 2)
			{
				safeMode = false;
				car.setColor(sf::Color::White);
			}
		}

		//Score and enemy speed increasing with time on playing game state 
		if (currentState == PLAYING)
		{
			score += 1 * dt * 100;
			enemyspeed = 850 + score * 0.3;

			// Update high score
			if (score > highScore)
			{
				highScore = score;
				ofstream writeFile("highscore.txt");
				if (writeFile.is_open())
				{
					writeFile << highScore;
				}
				writeFile.close();
			}
		}

		//Event loop
		while (window.pollEvent(event))
		{
			//*************************Enter name  State
			if (currentState == ENTERNAME)
			{
				if (event.type == sf::Event::TextEntered)
				{
					// Backspace dabane sa erase last character 
					if (event.text.unicode == 8)
					{
						if (!playerName.empty())
						{
							playerName.pop_back();
						}
					}

					// Normal letters only
					else if (event.text.unicode >= 32 &&
						event.text.unicode <= 126 &&
						playerName.length() < 12)
					{
						playerName += static_cast<char>(event.text.unicode);
					}
				}
			}

			//On presscross button on window close the window
			if (event.type == sf::Event::Closed)
			{
				window.close();
			}

			//KEYBOARD EVENTS
			if (event.type == sf::Event::KeyPressed)
			{

				//Press any key to go to menu
				if (currentState == SPLASH && event.type == sf::Event::KeyPressed)
				{
					currentState = MENU;
				}

				//ECS to close the window
				if (event.key.code == sf::Keyboard::Escape)
				{
					window.close();
				}

				//Tab button ka function
				if (event.key.code == sf::Keyboard::Tab)
				{
					if (currentState == HELP ||
						currentState == HIGHSCORES ||
						currentState == OPTIONS ||
						currentState == CARSELECT ||
						currentState == ENTERNAME)
					{
						currentState = MENU;
					}
				}

				//Pause the game
				if (event.key.code == sf::Keyboard::P)
				{
					if (currentState == PLAYING)
					{
						currentState = PAUSED;
						bgMusic.pause();
					}
					else if (currentState == PAUSED)
					{
						currentState = PLAYING;
						bgMusic.play();
					}
				}
			}


				//MOUSE CLICKING EVENTS
			if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
				{

						sf::Vector2i mousePos = sf::Mouse::getPosition(window);  //to get the curret position of cursor

						//*************************MENU State
						if (currentState == MENU)
						{
							if (menuPlayButton.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)))
							{
								clickSound.play();
								currentState = ENTERNAME;

							}

							//Highscores button
							if (highscoreButton.getGlobalBounds().contains(static_cast<float>(mousePos.x),static_cast<float>(mousePos.y)))
							{
								clickSound.play();
								currentState = HIGHSCORES;
							}

							//Help button
							if (helpButton.getGlobalBounds().contains(static_cast<float>(mousePos.x),static_cast<float>(mousePos.y)))
							{
								clickSound.play();
								currentState = HELP;
							}

							//Options button
							if (optionsButton.getGlobalBounds().contains(static_cast<float>(mousePos.x),static_cast<float>(mousePos.y)))
							{
								clickSound.play();
								currentState = OPTIONS;
							}

							//Exit button

							if (menuExitButton.getGlobalBounds().contains(static_cast<float>(mousePos.x),static_cast<float>(mousePos.y)))
							{
								clickSound.play();
								window.close();
							}
						}

						//*************************Car selecion State
						//Car selection
						if (currentState == CARSELECT)
						{
							if (Car1.getGlobalBounds().contains(static_cast<float>(mousePos.x),static_cast<float>(mousePos.y)))
							{
								car.setTexture(car1texture);
								car.setTextureRect(sf::IntRect(0,0,car1texture.getSize().x,car1texture.getSize().y));
								car.setScale(0.27f, 0.27f);

								clickSound.play();
								currentState = PLAYING;
							}

							if (Car2.getGlobalBounds().contains(static_cast<float>(mousePos.x),static_cast<float>(mousePos.y)))
							{
								car.setTexture(car2texture);
								car.setTextureRect(sf::IntRect(0,0,car2texture.getSize().x,car2texture.getSize().y));
								car.setScale(0.47f, 0.47f);

								clickSound.play();
								currentState = PLAYING;
							}

							if (Car3.getGlobalBounds().contains(static_cast<float>(mousePos.x),static_cast<float>(mousePos.y)))
							{
								car.setTexture(car3texture);
								car.setTextureRect(sf::IntRect(0,0,car3texture.getSize().x,car3texture.getSize().y));
								car.setScale(0.52f, 0.52f);

								clickSound.play();
								currentState = PLAYING;
							}
						}
						
						//*************************Enter name State
						//Start button
						if (currentState == ENTERNAME)
						{
							if (startButton.getGlobalBounds().contains(
								static_cast<float>(mousePos.x),
								static_cast<float>(mousePos.y)))
							{
								cout << "START CLICKED" << endl;

								clickSound.play();
								currentState = CARSELECT;
							}
						}

						//*************************Paused game State
						//Resume button click
						if (currentState == PAUSED)
						{
							if (resumeButton.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)))
							{
								clickSound.play();
								currentState = PLAYING;
							}
						}


						//*************************Game Over State
						// This handles RESTART
						if (playButton.getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)))
						{
							if (currentState == GAMEOVER)
							{
								clickSound.play();
								currentState = PLAYING;               

								ResetGame(score, lives, car, enemies);                 // Reset clock to prevent a speed glitch
							}
						}

						if (currentState == GAMEOVER)
						{
							if (exitButton.getGlobalBounds().contains(static_cast<float>(mousePos.x),static_cast<float>(mousePos.y)))
							{
								clickSound.play();
								sf::sleep(sf::milliseconds(200));       //takay exit ka button ko press karne par sound sun payen pohir screen band ho
								window.close();
							}
						}

						//*************************Options State
						if (currentState == OPTIONS)
						{
							if (musicOnButton.getGlobalBounds().contains(static_cast<float>(mousePos.x),static_cast<float>(mousePos.y)))
							{
								clickSound.play();
								bgMusic.play();
							}

							if (musicOffButton.getGlobalBounds().contains(static_cast<float>(mousePos.x),static_cast<float>(mousePos.y)))
							{
								clickSound.play();
								bgMusic.pause();
							}

							if (volumeUpButton.getGlobalBounds().contains(static_cast<float>(mousePos.x),static_cast<float>(mousePos.y)))
							{
								clickSound.play();
								bgMusic.setVolume(bgMusic.getVolume() + 10);
							}

							if (volumeDownButton.getGlobalBounds().contains(static_cast<float>(mousePos.x),static_cast<float>(mousePos.y)))
							{
								clickSound.play();
								bgMusic.setVolume(bgMusic.getVolume() - 10);
							}
						}
			}

				//---------------------------------------------------- mouse and keyboard event sec END
			
		}

		//*************************Playing State
		
		//Left right Motion of player car
		if (currentState == PLAYING)
		{
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
			{
				car.move(-speed * dt, 0);
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
			{
				car.move(speed * dt, 0);
			}
		}




		//Setting boundaries so that the car does not go outside of the screen
		//Left boundary
		if (car.getPosition().x < 40)
		{
			car.setPosition(40, car.getPosition().y);
		}
		//Right boundary
		if (car.getPosition().x > window.getSize().x - 190)
		{
			car.setPosition(window.getSize().x - 190, car.getPosition().y);
		}

		//------------------------------------------------------TEXT UPDATES--------------------------------
		//Score increasing with time updated on screen
		scoreText.setString("Score: " + std::to_string(score));
		//Updating lives
		livesText.setString("Lives: " + std::to_string(lives));
		//Updating player name
		playerText.setString("Player:\n\n" + playerName);
		//displaying text on game over screen
		finalScoreText.setString("YOUR SCORE: " + to_string(score));
		//displaying text on game over screen
		highScoreText.setString("HIGH SCORE: " + to_string(highScore));

		//---------------------------------------Moving and spawning Enemies-------------
		//Moving and spawning Enemies loop
		for (int i = 0; i < enemies.size(); i++)
		{
			if (currentState == PLAYING)
			{
				enemies[i].move(0, enemyspeed * dt);
			}

			if (enemies[i].getPosition().y > 1080)
			{
				RespawnEnemy(enemies[i], enemies, lanes);
			}
		}
		//-----------------------------------------Collision Detection---------------------------
		
		HandleCollision(
			car,
			enemies,
			lives,
			safeMode,
			safeClock,
			spark,
			showSpark,
			sparkClock,
			crashSound,
			currentState
		);

		//------------------------------------------------------Drawing part------------------------------------
		window.clear();

		//SPLASH SCREEN
		if (currentState == SPLASH)
		{
			SplashDummy();

			window.draw(overlay);

			window.draw(splashTitle);
			window.draw(presentedBy);
			window.draw(namesText);
			window.draw(continueText);

			window.display();
			continue;
		}

		//Draw Road 
		window.draw(road);
		window.draw(road2);


		//Menu Drawing
		if (currentState == MENU)
		{
			window.draw(overlay);
			window.draw(titleShadow);
			window.draw(gameTitle);
			window.draw(menuPlayButton);
			window.draw(menuPlayText);
			window.draw(highscoreButton);
			window.draw(helpButton);
			window.draw(optionsButton);
			window.draw(menuExitButton);
			window.draw(highscoreText);
			window.draw(helpText);
			window.draw(optionsText);
			window.draw(menuExitText);

			window.display();
			continue;
		}

		//Enter Name Screen 
		if (currentState == ENTERNAME)
		{
			//Player name 
			sf::Text playerNameText = CreateText(font, playerName, 40, sf::Color::White, 710, 565);

			window.draw(overlay);
			window.draw(nameBox);
			window.draw(enterNameText);
			window.draw(playerNameText);
			window.draw(startButton);
			window.draw(startText);
			window.draw(backText);

			window.display();

			continue;
		}

		//Car Selection Screen
		if (currentState == CARSELECT)
		{
			window.draw(overlay);
			window.draw(selectCarText);
			window.draw(Car1);
			window.draw(Car2);
			window.draw(Car3);
			window.draw(border1);
			window.draw(border2);
			window.draw(border3);
			window.draw(backText);

			window.display();

			continue;
		}


		//HELP SCREEN
		if (currentState == HELP)
		{
			window.draw(overlay);
			window.draw(overlay);
			window.draw(helpScreen);
			window.draw(backText);
			window.display();

			continue;
		}

		//HIGHSCORE SCREEN
		if (currentState == HIGHSCORES)
		{
			//High Score Text 
			sf::Text highscoreScreen = CreateText(
				font, "HIGH SCORE:\n\n" + to_string(highScore), 80, sf::Color::White, 550, 240);

			window.draw(overlay);
			window.draw(overlay);
			window.draw(highscoreScreen);
			window.draw(backText);
			window.display();
			continue;
		}

		//OPTIONS SCREEN
		if (currentState == OPTIONS)
		{
			window.draw(overlay);
			window.draw(overlay);
			window.draw(musicOnButton);
			window.draw(musicOffButton);
			window.draw(volumeUpButton);
			window.draw(volumeDownButton);
			window.draw(musicOnText);
			window.draw(musicOffText);
			window.draw(volumeUpText);
			window.draw(volumeDownText);
			window.draw(backText);

			window.display();

			continue;
		}
		
		if (currentState == PLAYING) 
		{
			//Drawing Enemies loop
			for (int i = 0; i < enemies.size(); i++)
			{
				window.draw(enemies[i]);
			}

			window.draw(scoreText);                                               //Drawing  Score
			window.draw(livesText);
			window.draw(playerText);
			window.draw(car);
		}

		//GAMEOVER SCREEN
		if (currentState == GAMEOVER )
		{
			window.draw(overlay);
			window.draw(overlay);
			window.draw(gameOverText);
			window.draw(finalScoreText);
			window.draw(highScoreText);
			window.draw(playButton);
			window.draw(Play);
			window.draw(exitButton);
			window.draw(Exit);
		}

		//PAUSED SCREEN
		if (currentState == PAUSED)
		{
			window.draw(overlay);
			window.draw(pauseText);
			window.draw(resumeButton);
			window.draw(resumeText);
			window.display();
			continue;
		}

		//Show spark for 0.2 sec
		if (showSpark)
		{
			window.draw(spark);

			if (sparkClock.getElapsedTime().asSeconds() > 0.2f)
			{
				showSpark = false;
			}
		}
		window.display();


	}
	return 0;
}