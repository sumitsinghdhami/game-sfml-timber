#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <sstream>
#include <algorithm>
using namespace sf;

// Function declaration
void updateBranches(int seed);

void setTextOnCentre(Text& text);

const int NUM_BRANCHES{ 6 };
Sprite branches[NUM_BRANCHES];

// Where is the player / branch ?
// Left or Right
enum class side { LEFT, RIGHT, NONE };
side branchPositions[NUM_BRANCHES];
int main() {
	String s;
	//Create a videomode object
	VideoMode vm(1920, 1080);

	//create and open a window for the game
	RenderWindow window(vm, " Timber!!!", Style::Fullscreen);

	// Create a texture to hold graphic on the GPU
	Texture textureBackground;

	// Load a graphic into the texture
	textureBackground.loadFromFile("graphics/background.png");

	// Create a sprite
	Sprite spriteBackground;

	// Attach the texture to the sprite
	spriteBackground.setTexture(textureBackground);

	// Set the sprite background to cover the screen
	spriteBackground.setPosition(0, 0);

	// Make a tree sprite 
	Texture textureTree;
	textureTree.loadFromFile("graphics/tree.png");
	Sprite spriteTree;
	spriteTree.setTexture(textureTree);
	spriteTree.setPosition(810, 0);

	//Prepare the bee
	Texture textureBee;
	textureBee.loadFromFile("graphics/bee.png");
	Sprite spriteBee;
	spriteBee.setTexture(textureBee);
	spriteBee.setPosition(0, 800);

	// Is the bee currently moving ?
	bool beeActive = false;

	// How fast the bee can fly ?
	float beeSpeed = 0.0f;

	// Make three cloud sprites from one texture
	Texture textureCloud;
	textureCloud.loadFromFile("graphics/cloud.png");
	Sprite spriteCloud[3];
	for (int i = 0; i < 3; i++) {
		spriteCloud[i].setTexture(textureCloud);
		spriteCloud[i].setPosition(0.0f, i * 100.0f);
	}

	// Are the clouds active ?
	bool cloudActive[3]{ false };

	//How fast is each cloud ?
	float cloudSpeed[3]{ 0.0f };
	// Variable to control time itself
	Clock clock;

	// Time Bar
	RectangleShape timeBar;
	float timeBarStartWidth{ 600 };
	float timeBarHeight{ 80 };
	timeBar.setSize(Vector2f(timeBarStartWidth, timeBarHeight));
	timeBar.setFillColor(Color::Green); // Changed initial color to Green from Red 
	timeBar.setPosition((1920.0f / 2) - timeBarStartWidth / 2, 980.0f);

	Time gameTimeTotal;
	float timeRemaining{ 6.0f };
	float timeBarWidthPerSecond = timeBarStartWidth / timeRemaining;
	// Track whether the game is running
	bool paused{ true };

	// Draw some text
	int score{ 0 };

	Text messageText;
	Text scoreText;

	// We need to choose a font
	Font font;
	font.loadFromFile("fonts/KOMIKAP_.ttf");

	// Set the font to our message
	messageText.setFont(font);
	scoreText.setFont(font);

	// Assign the actual message
	messageText.setString("Press Enter to start!");
	scoreText.setString(" Score = 0");

	// Set text font size
	messageText.setCharacterSize(75);
	scoreText.setCharacterSize(100);

	// Choose a color
	messageText.setFillColor(Color::White);
	scoreText.setFillColor(Color::White);

	// Position the text
	FloatRect textRect = messageText.getLocalBounds();

	messageText.setOrigin(textRect.left + textRect.width / 2.0f,
		textRect.top + textRect.height / 2.0f);

	messageText.setPosition(1920 / 2.0f, 1080 / 2.0f);

	scoreText.setPosition(40, 20);

	// Prepare 6 branches
	Texture textureBranch;
	textureBranch.loadFromFile("graphics/branch.png");

	// Set the texture for each branch sprite
	for (int i = 0; i < NUM_BRANCHES; i++) {
		branches[i].setTexture(textureBranch);
		branches[i].setPosition(-2000, -2000);

		// Set the sprtite's origin to dead centre
		// We can then spin it round without changing its position
		branches[i].setOrigin(220, 20);
	}

	// Prepare the player
	Texture texturePlayer;
	texturePlayer.loadFromFile("graphics/player.png");
	Sprite spritePlayer;
	spritePlayer.setTexture(texturePlayer);
	spritePlayer.setPosition(580, 720);

	// The player starts on the left
	side playerSide{ side::LEFT };

	// Prepare the gravestone
	Texture textureRip;
	textureRip.loadFromFile("graphics/rip.png");
	Sprite spriteRIP;
	spriteRIP.setTexture(textureRip);
	spriteRIP.setPosition(600, 2000);

	// Prepare the axe
	Texture textureAxe;
	textureAxe.loadFromFile("graphics/axe.png");
	Sprite spriteAxe;
	spriteAxe.setTexture(textureAxe);
	spriteAxe.setPosition(880, 830);
	spriteAxe.setRotation(180);

	// Line the axe up with the tree
	const float	AXE_POSITION_LEFT{ 880 };
	const float	AXE_POSITION_RIGHT{ 1075 };

	// Prepare the flying log
	Texture textureLog;
	textureLog.loadFromFile("graphics/log.png");
	Sprite spriteLog;
	spriteLog.setTexture(textureLog);
	spriteLog.setPosition(810, 720);

	// Some other useful log related variables
	bool logActive{ false };
	float logSpeedX{ 1000 };
	float logSpeedY{ -1500 };

	// Control the player's input
	bool acceptInput{ false };

	// Prepare the sounds
	// The player chopping sound
	SoundBuffer chopBuffer;
	chopBuffer.loadFromFile("sound/chop.wav");
	Sound chop;
	chop.setBuffer(chopBuffer);

	// The player has met his end under the branch
	SoundBuffer deathBuffer;
	deathBuffer.loadFromFile("sound/death.wav");
	Sound death;
	death.setBuffer(deathBuffer);

	// Out of time
	SoundBuffer ootBuffer;
	ootBuffer.loadFromFile("sound/oot.wav");
	Sound outOfTime;
	outOfTime.setBuffer(ootBuffer);

	// temporary variable to check if score increased
	int tempScore{ 0 };

	// Gameloop
	while (window.isOpen()) {
		/*
		****************************************
		Handle the players input
		****************************************
		*/

		Event event;
		while (window.pollEvent(event)) {
			if (event.type == Event::KeyReleased && !paused) {
				// Listen for key pressed again
				acceptInput = true;

				// hide the axe
				spriteAxe.setPosition(2100, spriteAxe.getPosition().y);

			}
		}
		if (Keyboard::isKeyPressed(Keyboard::Escape)) {
			window.close();
		}

		// Setting up a new game
		if (Keyboard::isKeyPressed(Keyboard::Enter) && paused) {
			paused = false;

			// Reset the score and timeRemaining
			score = 0;
			tempScore = -1;
			timeRemaining = 6.0f;
			timeBar.setFillColor(Color::Green); // Extra

			// Make all branches disappear -
			// starting in the second position
			for (int i = 0; i < NUM_BRANCHES; i++) {
				branchPositions[i] = side::NONE;
			}

			// Makes sure the gravestone is heidden
			spriteRIP.setPosition(675, 2000);

			// Move the player into position
			spritePlayer.setPosition(580, 720);

			acceptInput = true;

		}

		// Wrap the player controls to make sure we are accepting input
		if (acceptInput) {

			// First handle the right cursor key
			if (Keyboard::isKeyPressed(Keyboard::Right)) {
				// Make sure the player is on the right
				playerSide = side::RIGHT;
				score++;

				// Add to the amount of time remaining
				timeRemaining += 0.15f;

				spriteAxe.setPosition(AXE_POSITION_RIGHT, spriteAxe.getPosition().y);
				spriteAxe.setRotation(0);
				spritePlayer.setPosition(1200, 720);

				// Update the branches
				updateBranches(score);

				// Set the log flying to the left
				spriteLog.setPosition(810, 720);
				logSpeedX = -5000;
				logActive = true;
				acceptInput = false;

				// Play a chop sound
				chop.play();
			}

			if (Keyboard::isKeyPressed(Keyboard::Left)) {
				playerSide = side::LEFT;
				score++;

				// Add to the amount of time remaining
				timeRemaining += 0.15f;
				timeRemaining = std::min(6.0f, timeRemaining);
				spriteAxe.setPosition(AXE_POSITION_LEFT, spriteAxe.getPosition().y);
				spriteAxe.setRotation(180);
				spritePlayer.setPosition(580, 720);

				// Update the branches
				updateBranches(score);

				// Set the log flying to the right
				spriteLog.setPosition(810, 720);
				logSpeedX = 5000;
				logActive = true;
				acceptInput = false;

				// Play a chopping sound
				chop.play();
			}

		}

		/*
		****************************************
		Update the scene
		****************************************
		*/


		Time dt = clock.restart(); // dt -> delta time (time elapsed between two updates)
		if (!paused) {

			// Subtract from the amount of time remaining
			timeRemaining -= dt.asSeconds();

			// Size up the time bar
			timeBar.setSize(Vector2f(timeBarWidthPerSecond * timeRemaining, timeBarHeight));
			if (timeBar.getLocalBounds().width - timeBar.getLocalBounds().left <= timeBarStartWidth / 2.0f) { // Extra
				timeBar.setFillColor(Color::Red);
			}
			if (timeRemaining <= 0.0f) {
				paused = true;
				acceptInput = false;
				// Change the message shown to the player
				messageText.setString("Out of time! Press Enter to play again!");

				// Position the message text based on its new size
				setTextOnCentre(messageText);

				// Play the out of time sound
				outOfTime.play();
			}
			// Set up the bee
			if (!beeActive) {

				// How fast is the bee
				srand((int)time(0));
				beeSpeed = (rand() % 200) + 200.0f;

				// How high is the bee
				srand((int)time(0) * 10);
				float height = (rand() % 500) + 500.0f;
				spriteBee.setPosition(2000, height);
				beeActive = true;
			}
			// Move the bee
			else {
				spriteBee.setPosition(
					spriteBee.getPosition().x - (beeSpeed * dt.asSeconds()),
					spriteBee.getPosition().y);

				// Has the bee reached the left-hand edge of the screen ?
				if (spriteBee.getPosition().x < -100) {
					// Set it up ready to be a whole new bee next frame
					beeActive = false;
				}
			}

			// Manage the clouds
			for (int i = 0; i < 3; i++) {
				if (!cloudActive[i]) {

					// How fast is the cloud ?
					srand((int)time(0) * i * 10);
					cloudSpeed[i] = (float)(rand() % ((i + 1) * 100));

					// How high is the cloud
					srand((int)time(0) * 10);
					float height = (float)(rand() % ((i + 1) * 150));
					spriteCloud[i].setPosition(-250, height);
					cloudActive[i] = true;
				}
				else {
					spriteCloud[i].setPosition(
						spriteCloud[i].getPosition().x + (cloudSpeed[i] * dt.asSeconds()),
						spriteCloud[i].getPosition().y);
					// Has the cloud reached the right hand edge of the screen ?
					if (spriteCloud[i].getPosition().x > 1920) {
						// Set it up ready to be a whole new cloud next frame
						cloudActive[i] = false;
					}
				}
			}

			// Update the score text
			if (score > tempScore ) {
				tempScore = score;
				std::stringstream ss;
				ss << "Score = " << score;
				scoreText.setString(ss.str());
			}

			// Update the branches
			for (int i = 0; i < NUM_BRANCHES; i++) {
				float height = i * 150.0f;
				if (branchPositions[i] == side::LEFT) {
					// Move the sprite to the left side
					branches[i].setPosition(610, height);

					// Flip the sprite round the other way
					branches[i].setRotation(180);
				}
				else if (branchPositions[i] == side::RIGHT) {
					// Move the sprite to the right side
					branches[i].setPosition(1330, height);

					// Set the sprite rotation to normal
					branches[i].setRotation(0);
				}
				else {
					// Hide the branch
					branches[i].setPosition(3000, height);

				}
			}

			// Handle a flying log
			if (logActive) {
				spriteLog.setPosition(spriteLog.getPosition().x + (logSpeedX * dt.asSeconds()),
					spriteLog.getPosition().y + (logSpeedY * dt.asSeconds()));
				// Has the log reached the right hand edge ? 
				if (spriteLog.getPosition().x < -100 || spriteLog.getPosition().x > 2000) {
					// set it up to be a whole new log next frame
					logActive = false;
					spriteLog.setPosition(810, 720);
				}
			}

			// Has the player beeen squished by a branch ?
			if (branchPositions[5] == playerSide) {

				// Death
				paused = true;
				acceptInput = false;

				// Draw the gravestone 
				spriteRIP.setPosition(525, 760);

				// Hide the player
				spritePlayer.setPosition(2000, 660);

				// change the text of the message 
				messageText.setString("SQUISHED!!");

				// Center it on the screen
				setTextOnCentre(messageText);

				// Play the death sound
				death.play();
			}

		}

		/*
		****************************************
		Draw the scene
		****************************************
		*/

		// Clear everything from last frame
		window.clear();

		// Draw the game scene here
		window.draw(spriteBackground);

		// Draw the clouds
		for (size_t i = 0; i < 3; i++)	window.draw(spriteCloud[i]);

		// Draw the branches
		for (int i = 0; i < NUM_BRANCHES; i++) 	window.draw(branches[i]);

		// Draw the flying log
		if (!paused)
			window.draw(spriteLog);

		// Draw the tree
		window.draw(spriteTree);

		// Draw the player
		window.draw(spritePlayer);

		//Draw the axe
		window.draw(spriteAxe);



		// Draw the gravestone
		window.draw(spriteRIP);

		// Draw the bee
		window.draw(spriteBee);

		// Draw the score
		window.draw(scoreText);

		// Draw Time Bar
		window.draw(timeBar);
		
		if (paused) {
			// Draw our message 
			window.draw(messageText);
		}

		//Show everything we just drew
		window.display();
	}
	return 0;
}

void updateBranches(int seed) {

	// Move all the branches down one place
	for (int i = NUM_BRANCHES - 1; i > 0; i--) {
		branchPositions[i] = branchPositions[i - 1];
	}
	// Spawn a new branch at position 0
	// LEFT, RIGHT, NONE
	srand((int)time(0) + seed);
	int r = (rand() % 5);
	switch (r) {
	case 0:
		branchPositions[0] = side::LEFT;
		break;
	case 1:
		branchPositions[0] = side::RIGHT;
		break;
	default:
		branchPositions[0] = side::NONE;
		break;
	}

}

void setTextOnCentre(Text& text) {
	FloatRect textRect = text.getLocalBounds();
	text.setOrigin(textRect.left + textRect.width / 2,
		textRect.top + textRect.height / 2);
}