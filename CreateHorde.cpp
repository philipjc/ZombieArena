#include "ZombieArena.h"
#include "Zombie.h"

/*
	Zombie* createHorde:
	
	Notice that the function must return a pointer to a Zombie object.
	We will be creating an array of Zombie objects. Once we are done creating the horde, 
	we will return the array. When we return the array, we are actually returning the 
	address of the first element of the array. This, as we learned in the section
	on pointers earlier in this chapter, is the same thing as a pointer. 
	
	The signature also shows that we have two parameters. The first, numZombies, will be the 
	number of zombies this current horde requires and the second, arena, is an IntRect
	that holds the size of the current arena in which to create this horde.

	After the function signature, we declare a pointer to the Zombie type called zombies
	and initialize it with the memory address of the first element of an array, which we 
	dynamically allocate on the heap:
*/

Zombie* createHorde(int numZombies, IntRect arena)
{
	Zombie* zombies = new Zombie[numZombies];

	/*
		The next part of the code simply copies the extremities of the arena into maxY, minY, 
		maxX, and minX. We subtract twenty pixels from the right and bottom while adding 
		twenty pixels to the top and left. We use these four local variables to help position 
		each of the zombies. We made the twenty-pixel adjustments to stop the zombies 
		appearing on top of the walls:
	*/

	int maxY = arena.height - 20;
	int minY = arena.top + 20;

	int maxX = arena.width - 20;
	int minX = arena.left + 20;

	for (int i = 0; i < numZombies; i++)
	{

		// Which side should the zombie spawn
		srand((int)time(0) * i);

		int side = (rand() % 4);

		float x, y;

		switch (side)
		{
		case 0:
			// left
			x = minX;
			y = (rand() % maxY) + minY;

			break;
		case 1:
			// right
			x = maxX;
			y = (rand() % maxY) + minY;

			break;
		case 2:
			// top
			x = (rand() % maxX) + minX;
			y = minY;

			break;
		case 3:
			// bottom
			x = (rand() % maxX) + minX;
			y = maxY;

			break;
		}

		// Bloater, crawler or runner
		srand((int)time(0) * i * 2);

		int type = (rand() % 3);

		// Spawn the new zombie into the array
		zombies[i].spawn(x, y, type, i);

	}

	return zombies;
}
