// ZombieArena

#include <SFML/Graphics.hpp>
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

	// The main game loop
	while (window.isOpen())
	{
		/*
		 ************
		 Handle input
		 ************
		 */

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
						}
						else if (bulletsSpare > 0)
						{
							// Only few bullets left
							bulletsInClip = bulletsSpare;
							bulletsSpare = 0;
						}
						else // no spare bullets
						{
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
				state = State::PLAYING;
			}
			if (event.key.code == Keyboard::Num2)
			{
				state = State::PLAYING;
			}
			if (event.key.code == Keyboard::Num3)
			{
				state = State::PLAYING;
			}
			if (event.key.code == Keyboard::Num4)
			{
				state = State::PLAYING;
			}
			if (event.key.code == Keyboard::Num5)
			{
				state = State::PLAYING;
			}
			if (event.key.code == Keyboard::Num6)
			{
				state = State::PLAYING;
			}

			if (state == State::PLAYING)
			{
				// Prepare the level (arena tiles)
				arena.width = 500;
				arena.height = 500;
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
				numberOfZombies = 10;

				// Delete the previously allocated memory (if it exists)
				delete[] zombies;
				zombies = createHorde(numberOfZombies, arena);

				numberOfZombiesAlive = numberOfZombies;

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
					Bullet bulletToCheck = bullets[i];
					Zombie zombieTocheck = zombies[j];

					bool bulletInFlight = bulletToCheck.isInFlight();
					bool zombieHasHitPoints = zombieTocheck.isAlive();

					if (bulletInFlight && zombieHasHitPoints)
					{
						bool bulletHitZombie = bulletToCheck.getPosition().intersects(zombieTocheck.getPosition());

						if (bulletHitZombie)
						{
							// Stop the bullet
							bulletToCheck.stop();

							// Register the hit and see if it was a kill
							if (zombieTocheck.hit())
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
							}

						}
					}
				}
			} // End zombie being shot

			// Zombie Player Collision 
			for (int i = 0; i < numberOfZombies; i++)
			{
				FloatRect playerPosition = player.getPosition();
				FloatRect zombiePosition = zombies[i].getPosition();
				bool zombieAlive = zombies[i].isAlive();

				if (playerPosition.intersects(zombiePosition) && zombieAlive)
				{
					if (player.hit(gameTimeTotal))
					{
						// More here later
					}
					if (player.getHealth() <= 0)
					{
						state = State::GAME_OVER;
					}
				}
			} // End player touched

			// Has the player touched health pickup
			if (player.getPosition().intersects(healthPickup.getPosition()) && healthPickup.isSpawned())
			{
				player.increaseHealthLevel(healthPickup.gotIt());

			}
			
			// Has the player touched ammo pickup
			if (player.getPosition().intersects(ammoPickup.getPosition()) && ammoPickup.isSpawned())
			{
				bulletsSpare += ammoPickup.gotIt();

			}

		} // End updating the scene

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
		}

		if (state == State::LEVELING_UP)
		{

		}

		if (state == State::PAUSED)
		{

		}

		if (state == State::GAME_OVER)
		{

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

