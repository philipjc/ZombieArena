// ZombieArena

#include <sstream>
#include <fstream>
#include "SFML/Audio.hpp"
#include "SFML/Main.hpp"
#include "ZombieArena.h"
#include "TextureHolder.h"
#include "Player.h"
#include "Bullet.h"
#include "Pickup.h"

using namespace sf;

int main()
{
	// TextureHolder Instance
	TextureHolder textureHolder;

	// The game will always be in one of four states
	enum class State {
		PAUSED, 
		LEVELING_UP,
		GAME_OVER,
		PLAYING
	};

	// Start with the GAME_OVER state
	State state = State::GAME_OVER;

	// Get the screen resolution and create an SFML window
	Vector2f resolution;
	resolution.x = VideoMode::getDesktopMode().width;
	resolution.y = VideoMode::getDesktopMode().height;

	RenderWindow window(VideoMode(resolution.x, resolution.y), "Zombie Arena", Style::Fullscreen);

	// Create a an SFML View for the main action
	View mainView(sf::FloatRect(0, 0, resolution.x, resolution.y));

	// Here is our clock for timing everything
	Clock clock;

	// How long has the PLAYING state been active
	Time gameTimeTotal;

	// Where is the mouse in relation to world coordinates
	Vector2f mouseWorldPosition;

	// Where is the mouse in relation to screen coordinates
	Vector2i mouseScreenPosition;

	// Create an instance of the Player class
	Player player;

	// The boundaries of the arena
	IntRect arena;

	// Create the background
	VertexArray background;

	// Load texture for the background vertex array
	Texture textureBackground = TextureHolder::GetTexture("graphics/background_sheet.png");

	// Prepare Zombie Horde
	int numberOfZombies;
	int numberOfZombiesAlive;
	Zombie* zombies = nullptr;

	// 100 bullets
	Bullet bullets[100];

	int currentBullet = 0;
	int bulletsSpare = 24;

	int bulletsInClip = 6;
	int clipSize = 6;

	float fireRate = 1;

	// When was the fire button last pressed?
	Time lastPressed;

	// Hide the mouse pointer and replace it with crosshair
	window.setMouseCursorVisible(true);

	Sprite spriteCrosshair;
	Texture textureCrosshair = TextureHolder::GetTexture("graphics/crosshair.png");

	spriteCrosshair.setTexture(textureCrosshair);
	spriteCrosshair.setOrigin(25, 25);

	// Pickups
	// Create a couple of pickups
	Pickup healthPickup(1);
	Pickup ammoPickup(2);

	// Game details
	int score = 0;
	int highScore = 0;

	// ============ UI ===============
	// ===============================
	// 
	// For the home/game over screen
	Sprite spriteGameOver;
	Texture textureGameOver = TextureHolder::GetTexture("graphics/background.png");
	spriteGameOver.setTexture(textureGameOver);
	spriteGameOver.setPosition(0, 0);

	// Create a view for the HUD
	View hudView(sf::FloatRect(0, 0, resolution.x, resolution.y));

	// Create a sprite for the ammo icon
	Sprite spriteAmmoIcon;
	Texture textureAmmoIcon = TextureHolder::GetTexture("graphics/ammo_icon.png");
	spriteAmmoIcon.setTexture(textureAmmoIcon);
	spriteAmmoIcon.setPosition(20, 980);

	// Load the font
	Font font;
	font.loadFromFile("fonts/zombiecontrol.ttf");

	// Paused
	Text pausedText;
	pausedText.setFont(font);
	pausedText.setCharacterSize(155);
	pausedText.setFillColor(Color::White);
	pausedText.setPosition(400, 400);
	pausedText.setString("Press Enter \nto continue");

	// Game Over
	Text gameOverText;
	gameOverText.setFont(font);
	gameOverText.setCharacterSize(125);
	gameOverText.setFillColor(Color::White);
	gameOverText.setPosition(250, 850);
	gameOverText.setString("Press Enter to play");

	// LEVELING up
	Text levelUpText;
	levelUpText.setFont(font);
	levelUpText.setCharacterSize(80);
	levelUpText.setFillColor(Color::White);
	levelUpText.setPosition(150, 250);

	std::stringstream levelUpStream;
	levelUpStream <<
		"1- Increased rate of fire" <<
		"\n2- Increased clip size(next reload)" <<
		"\n3- Increased max health" <<
		"\n4- Increased run speed" <<
		"\n5- More and better health pickups" <<
		"\n6- More and better ammo pickups";
	levelUpText.setString(levelUpStream.str());

	// Ammo
	Text ammoText;
	ammoText.setFont(font);
	ammoText.setCharacterSize(55);
	ammoText.setFillColor(Color::White);
	ammoText.setPosition(200, 980);

	// Score
	Text scoreText;
	scoreText.setFont(font);
	scoreText.setCharacterSize(55);
	scoreText.setFillColor(Color::White);
	scoreText.setPosition(20, 0);

	// Load the high score from a text file
	std::ifstream inputFile("gamedata/scores.txt");

	if (inputFile.is_open())
	{
		// >> Reads the data
		inputFile >> highScore;
		inputFile.close();
	}

	// Hi Score
	Text hiScoreText;
	hiScoreText.setFont(font);
	hiScoreText.setCharacterSize(55);
	hiScoreText.setFillColor(Color::White);
	hiScoreText.setPosition(1400, 0);

	std::stringstream s;
	s << "Hi Score:" << highScore;

	hiScoreText.setString(s.str());

	// Zombies remaining
	Text zombiesRemainingText;
	zombiesRemainingText.setFont(font);
	zombiesRemainingText.setCharacterSize(55);
	zombiesRemainingText.setFillColor(Color::White);
	zombiesRemainingText.setPosition(1500, 980);
	zombiesRemainingText.setString("Zombies: 100");

	// Wave number
	int wave = 0;
	Text waveNumberText;
	waveNumberText.setFont(font);
	waveNumberText.setCharacterSize(55);
	waveNumberText.setFillColor(Color::White);
	waveNumberText.setPosition(1250, 980);
	waveNumberText.setString("Wave: 0");

	// Health bar
	RectangleShape healthBar;
	healthBar.setFillColor(Color::Red);
	healthBar.setPosition(450, 980);

	// When did we last update the HUD?
	int framesSinceLastHUDUpdate = 0;

	// How often (in frames) should we update the HUD
	int fpsMeasurementFrameInterval = 1000;

	// Prepare the hit sound
	SoundBuffer hitBuffer;
	hitBuffer.loadFromFile("sound/hit.wav");
	Sound hit;
	hit.setBuffer(hitBuffer);

	// Prepare the splat sound
	SoundBuffer splatBuffer;
	splatBuffer.loadFromFile("sound/splat.wav");
	Sound splat;
	splat.setBuffer(splatBuffer);

	// Prepare the shoot sound
	SoundBuffer shootBuffer;
	shootBuffer.loadFromFile("sound/shoot.wav");
	Sound shoot;
	shoot.setBuffer(shootBuffer);

	// Prepare the reload sound
	SoundBuffer reloadBuffer;
	reloadBuffer.loadFromFile("sound/reload.wav");
	Sound reload;
	reload.setBuffer(reloadBuffer);

	// Prepare the failed sound
	SoundBuffer reloadFailedBuffer;
	reloadFailedBuffer.loadFromFile("sound/reload_failed.wav");
	Sound reloadFailed;
	reloadFailed.setBuffer(reloadFailedBuffer);

	// Prepare the powerup sound
	SoundBuffer powerupBuffer;
	powerupBuffer.loadFromFile("sound/powerup.wav");
	Sound powerup;
	powerup.setBuffer(powerupBuffer);

	// Prepare the pickup sound
	SoundBuffer pickupBuffer;
	pickupBuffer.loadFromFile("sound/pickup.wav");
	Sound pickup;
	pickup.setBuffer(pickupBuffer);


	// The main game loop
	while (window.isOpen())
	{
		/*
		�************
		�Handle input
		�************
		�*/

	// Handle events by polling
		Event event;

		while (window.pollEvent(event))
		{
			if (event.type == Event::KeyPressed)
			{
				// Pause a game while playing
				if (event.key.code == Keyboard::Return && state == State::PLAYING)
				{
					state = State::PAUSED;
				}
				// Restart while paused
				else if (event.key.code == Keyboard::Return && state == State::PAUSED)
				{
					state = State::PLAYING;
					// Reset the clock so there isn't a frame jump
					clock.restart();
				}
				// Start a new game while in GAME_OVER state
				else if (event.key.code == Keyboard::Return && state == State::GAME_OVER)
				{
					state = State::LEVELING_UP;

					wave = 0;
					score = 0;

					// Prepare the gun and ammo for next game
					currentBullet = 0;
					bulletsSpare = 24;
					bulletsInClip = 6;
					clipSize = 6;
					fireRate = 1;

					// Reset the player's stats
					player.resetPlayerStats();

				}
				if (state == State::PLAYING)
				{
					// Reloading
					if (event.key.code == Keyboard::R)
					{
						if (bulletsSpare >= clipSize)
						{
							// Plenty of bullets. Reload.
							bulletsInClip = clipSize;
							bulletsSpare -= clipSize;
							reload.play();
						}
						else if (bulletsSpare > 0)
						{
							// Only few bullets left
							bulletsInClip = bulletsSpare;
							bulletsSpare = 0;
							reload.play();
						}
						else // no spare bullets
						{
							reloadFailed.play();
							// More here soon?!
						}
					}
				}
			}
		} // End event polling

		// Handle the player quitting
		if (Keyboard::isKeyPressed(Keyboard::Escape))
		{
			window.close();
		}

		// Handle WASD while playing
		if (state == State::PLAYING)
		{
			// Handle the pressing and releasing of the WASD keys
			if (Keyboard::isKeyPressed(Keyboard::W))
			{
				player.moveUp();
			}
			else
			{
				player.stopUp();
			}

			if (Keyboard::isKeyPressed(Keyboard::S))
			{
				player.moveDown();
			}
			else
			{
				player.stopDown();
			}

			if (Keyboard::isKeyPressed(Keyboard::A))
			{
				player.moveLeft();
			}
			else
			{
				player.stopLeft();
			}

			if (Keyboard::isKeyPressed(Keyboard::D))
			{
				player.moveRight();
			}
			else
			{
				player.stopRight();
			}

			// Fire a bullet
			if (Mouse::isButtonPressed(sf::Mouse::Left))
			{
				/*
					All the previous code is wrapped in an if statement that executes whenever the left 
					mouse button is pressed, that is, if (Mouse::isButtonPressed(sf::Mouse::Le
					ft)). Note that the code will execute repeatedly, even if the player just holds down 
					the button. The code we will go through now controls the rate of fire.
					In the preceding code, we then check whether the total time elapsed in the game 
					(gameTimeTotal) minus the time the player last shot a bullet (lastPressed) is 
					greater than 1,000, divided by the current rate of fire and that the player has at 
					least one bullet in the clip. We use 1,000 because this is the number of milliseconds 
					in a second.

					If this test is successful, the code that actually fires a bullet is executed. Shooting a 
					bullet is easy because we did all the hard work in the Bullet class. We simply call 
					shoot on the current bullet from the bullets array. We pass in the player's and the 
					cross-hair's current horizontal and vertical locations. The bullet will be configured 
					and set in flight by the code in the shoot function of the Bullet class.

					All we must do is keep track of the array of bullets. We incremented the 
					currentBullet variable. Then, we need to check to see whether we fired the last 
					bullet (99) with the if (currentBullet > 99) statement. If it was the last bullet, we 
					set currentBullet to zero. If it wasn't the last bullet, then the next bullet is ready to 
					go whenever the rate of fire permits it and the player presses the left mouse button.

					Finally, in the preceding code, we store the time that the bullet was fired into 
					lastPressed and decrement bulletsInClip
				*/

				if (gameTimeTotal.asMilliseconds() - lastPressed.asMilliseconds() > 1000 / fireRate && bulletsInClip > 0)
				{
					// Pass the centre of the player 
					// and the centre of the cross-hair
					// to the shoot function
					bullets[currentBullet].shoot(player.getCenter().x, player.getCenter().y, mouseWorldPosition.x, mouseWorldPosition.y);

					currentBullet++;

					if (currentBullet > 99)
					{
						currentBullet = 0;
					}

					lastPressed = gameTimeTotal;

					shoot.play();

					bulletsInClip--;
				}

			} // End fire a bullet

		} // End WASD while playing

		// Handle the LEVELING up state
		if (state == State::LEVELING_UP)
		{
			// Handle the player LEVELING up
			if (event.key.code == Keyboard::Num1)
			{
				// Increase fire rate
				fireRate++;
				state = State::PLAYING;
			}
			if (event.key.code == Keyboard::Num2)
			{
				// Increase clip size
				clipSize++;
				state = State::PLAYING;
			}
			if (event.key.code == Keyboard::Num3)
			{
				// Increase health
				player.upgradeHealth();
				state = State::PLAYING;
			}
			if (event.key.code == Keyboard::Num4)
			{
				// Increase speed
				player.upgradeSpeed();
				state = State::PLAYING;
			}
			if (event.key.code == Keyboard::Num5)
			{
				// Upgrade pickup
				healthPickup.upgrade();
				state = State::PLAYING;
			}
			if (event.key.code == Keyboard::Num6)
			{
				// Upgrade pickup
				ammoPickup.upgrade();
				state = State::PLAYING;
			}

			if (state == State::PLAYING)
			{
				// Prepare the level (arena tiles)
				arena.width = 1000;
				arena.height = 1000;
				arena.left = 0;
				arena.top = 0;

				// Pass the vertex array background by reference .
				// What happens in createBackground actual happens to background here in the main loop.
				int tileSize = createBackground(background, arena);

				// We will modify this line of code later. Not need now we have createBackground
				// int tileSize = 50;

				// Spawn the player in the middle of the arena
				player.spawn(arena, resolution, tileSize);

				// Configure the pickups
				// Now knows where they can spawn. 
				healthPickup.setArena(arena);
				ammoPickup.setArena(arena);

				// Create a horde of zombies
				numberOfZombies = 5;

				// Delete the previously allocated memory (if it exists)
				delete[] zombies;
				zombies = createHorde(numberOfZombies, arena);

				numberOfZombiesAlive = numberOfZombies;

				// play powerup sound
				powerup.play();

				// Reset the clock so there isn't a frame jump
				clock.restart();
			}
		} // End LEVELING up

		if (state == State::PLAYING)
		{

			/*
				****************
				UPDATE THE FRAME
				****************
			
				Now, we have an actual spawned player object that is aware of its 
				environment and can respond to key presses. We can now update the 
				scene on each pass through the loop.
			*/

			// Update the delta time
			Time dt = clock.restart();

			// Update the total game time
			gameTimeTotal += dt;

			// Make a decimal fraction of 1 from the delta time
			float dtAsSeconds = dt.asSeconds();

			// Where is the mouse pointer
			mouseScreenPosition = Mouse::getPosition();

			// Convert mouse position to the world coordinates of mainView view
			mouseWorldPosition = window.mapPixelToCoords(Mouse::getPosition(), mainView);

			// ===================================================================================
			// ===========================  REFACTOR OPPORTUNITY  ================================
			// 
			//  An interesting improvement to the game would be to add a small random amount of 
			// inaccuracy to each shot.This inaccuracy could perhaps be mitigated with an upgrade
			//	between waves.
			// ===================================================================================
			
			// Set the crosshair to the mouse world location
			spriteCrosshair.setPosition(mouseWorldPosition);

			// Update the player using the dt seconds and a static function from Mouse Class.
			player.update(dtAsSeconds, Mouse::getPosition());

			// Make a note of the players new position (not yet used)
			Vector2f playerPosition(player.getCenter());

			// Make the view centre around the player 
			mainView.setCenter(player.getCenter());

			// Loop through each Zombie and update them
			for (int i = 0; i < numberOfZombies; i++)
			{
				if (zombies[i].isAlive())
				{
					zombies[i].update(dt.asSeconds(), playerPosition);
				}
			}

			// Update any bullets that are in-flight
			// We made 100 bullets, update them all.
			for (int i = 0; i < 100; i++)
			{
				if (bullets[i].isInFlight())
				{
					bullets[i].update(dtAsSeconds);
				}
			}


			// Update health
			healthPickup.update(dtAsSeconds);

			// Update ammo
			ammoPickup.update(dtAsSeconds);

			// Collision detection
			// ===================
			// 
			// Shoot Zombie
			for (int i = 0; i < 100; i++)
			{
				for (int j = 0; j < numberOfZombies; j++)
				{
					
					if (bullets[i].isInFlight() && zombies[j].isAlive())
					{

						if (bullets[i].getPosition().intersects(zombies[j].getPosition()))
						{
							// Stop the bullet
							bullets[i].stop();

							// Register the hit and see if it was a kill
							if (zombies[j].hit())
							{
								// Adjust the score
								score += 10;

								if (score >= highScore)
								{
									highScore = score;
								}

								// Kill zombie
								numberOfZombies--;

								// When all the zombies are dead (again)
								if (numberOfZombiesAlive == 0) {
									state = State::LEVELING_UP;
								}

								splat.play();
							}

						}
					}
				}
			} // End zombie being shot

			// Zombie Player Collision 
			for (int i = 0; i < numberOfZombies; i++)
			{

				if (player.getPosition().intersects(zombies[i].getPosition()) && zombies[i].isAlive())
				{
					if (player.hit(gameTimeTotal))
					{
						hit.play();
						// More here later
					}
					if (player.getHealth() <= 0)
					{
						state = State::GAME_OVER;

						std::ofstream outputFile("gamedata/scores.txt");

						// << writes the data
						outputFile << highScore;
						outputFile.close();
					}
				}
			} // End player touched

			// Has the player touched health pickup
			if (player.getPosition().intersects(healthPickup.getPosition()) && healthPickup.isSpawned())
			{
				player.increaseHealthLevel(healthPickup.gotIt());
				pickup.play();

			}
			
			// Has the player touched ammo pickup
			if (player.getPosition().intersects(ammoPickup.getPosition()) && ammoPickup.isSpawned())
			{
				bulletsSpare += ammoPickup.gotIt();
				reload.play();
			}

			// size up the health bar
			healthBar.setSize(Vector2f(player.getHealth() * 3, 50));

			// Increment the number of frames since the previous update
			framesSinceLastHUDUpdate++;

			// re-calculate every fpsMeasurementFrameInterval frames
			if (framesSinceLastHUDUpdate > fpsMeasurementFrameInterval)
			{
				// Update game HUD text
				std::stringstream ssAmmo;
				std::stringstream ssScore;
				std::stringstream ssHiScore;
				std::stringstream ssWave;
				std::stringstream ssZombiesAlive;

				// Update the ammo text
				ssAmmo << bulletsInClip << "/" << bulletsSpare;
				ammoText.setString(ssAmmo.str());

				// Update the score text
				ssScore << "Score:" << score;
				scoreText.setString(ssScore.str());

				// Update the high score text
				ssHiScore << "Hi Score:" << highScore;
				hiScoreText.setString(ssHiScore.str());

				// Update the wave
				ssWave << "Wave:" << wave;
				waveNumberText.setString(ssWave.str());

				// Update the high score text
				ssZombiesAlive << "Zombies:" << numberOfZombiesAlive;
				zombiesRemainingText.setString(ssZombiesAlive.str());
				framesSinceLastHUDUpdate = 0;


		} // End UPDATING the scene

		/*
			**************
			Draw the scene
			**************
		*/
			if (state == State::PLAYING)
			{
				window.clear();

				// set the mainView to be displayed in the window
				// And draw everything related to it
				window.setView(mainView);

				// Draw background (arean tiles)
				window.draw(background, &textureBackground);

				// Draw the zombies
				for (int i = 0; i < numberOfZombies; i++)
				{
					// We don't check whether the zombie is alive because even
					// if the zombie is dead, we want to draw the blood splatter.
					window.draw(zombies[i].getSprite());
				}


				// Draw the bullets
				for (int i = 0; i < 100; i++)
				{
					if (bullets[i].isInFlight())
					{
						window.draw(bullets[i].getShape());
					}
				}

				// Draw the player
				window.draw(player.getSprite());

				// Draw the pick-ups, if currently spawned
				if (ammoPickup.isSpawned())
				{
					window.draw(ammoPickup.getSprite());
				}

				if (healthPickup.isSpawned())
				{
					window.draw(healthPickup.getSprite());
				}

				// Draw the crosshair
				window.draw(spriteCrosshair);

				// Switch to the HUD view
				window.setView(hudView);

				// Draw all the HUD elements
				// ==========================
				// The vital thing to notice in this block of code is that we switch views to the 
				// HUD view.This causes everything to be drawn at the precise screen positions we
				// gave each of the elements of the HUD.They will never move.
				window.draw(spriteAmmoIcon);
				window.draw(ammoText);
				window.draw(scoreText);
				window.draw(hiScoreText);
				window.draw(healthBar);
				window.draw(waveNumberText);
				window.draw(zombiesRemainingText);
			}

		}

		if (state == State::LEVELING_UP)
		{
			window.draw(spriteGameOver);
			window.draw(levelUpText);
		}

		if (state == State::PAUSED)
		{
			window.draw(pausedText);
		}

		if (state == State::GAME_OVER)
		{
			window.draw(spriteGameOver);
			window.draw(gameOverText);
			window.draw(scoreText);
			window.draw(hiScoreText);
		}
		
		window.display();

	} // End game loop

	/*
		At the end of the main function, we need to make sure to delete our pointer because 
		it is a good practice as well as often being essential. However, technically, this isn't 
		essential because the game is about to exit, and the operating system will reclaim all 
		the memory that's used after the return 0 statement:
	*/

	// Delete the previously allocated memory (if it exists)
	delete[] zombies;

	return 0;
}

