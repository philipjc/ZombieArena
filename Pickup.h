#pragma once

#include <SFML/Graphics.hpp>

using namespace sf;

class Pickup
{
private:
	//Start value for health pickups
	const int HEALTH_START_VALUE = 50;
	const int AMMO_START_VALUE = 12;

	const int START_WAIT_TIME = 10;
	const int START_SECONDS_TO_LIVE = 5;

	// The sprite that represents this pickup
	Sprite m_Sprite;

	// The arena it exists in. Holds size of arena so we know where to spawn.
	IntRect m_Arena;

	// How much is this pickup worth?
	int m_Value;

	// What type of pickup is this? 
	// 1 = health, 2 = ammo
	int m_Type;

	// =========================  DESIGN  INFO  =========================
	// Adding complexity through more variables makes pickups 
	// more interesting, and requires the player to think of
	// a strategy rather than just pickup items that lie around forever.
	// ==================================================================

	// Handle spawning and disappearing
	bool m_Spawned;
	float m_SecondsSinceSpawn;
	float m_SecondsSinceDeSpawn;

	// Time to until despawn
	float m_SecondsToLive;

	// Despawned time
	float m_SecondsToWait;

	// Public prototypes go here

public:
	
	Pickup(int type);

	// Prepare a new pickup
	void setArena(IntRect arena); // Item knows where it is in the arena.
	void spawn();

	// Check the position of a pickup
	FloatRect getPosition();

	// Get the sprite for drawing
	Sprite getSprite();

	// Let the pickup update itself each frame. Spawns or de-spawns
	void update(float elapsedTime);

	// Is this pickup currently spawned?
	bool isSpawned();

	// Get the goodness from the pickup. Know if its been collided with
	int gotIt();

	// Upgrade the value of each pickup
	void upgrade();

};
