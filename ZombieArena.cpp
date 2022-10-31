// ZombieArena

#include <SFML/Graphics.hpp>
#include "ZombieArena.h"
#include "TextureHolder.h"
#include "Player.h"

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

			// Draw the player
			window.draw(player.getSprite());
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
