#pragma once
#include "Zombie.h"

#include <SFML/Graphics.hpp> // added tis due to code errors, but maybe once used in main take other from Player?

using namespace sf;

int createBackground(VertexArray& rVA, IntRect arena);

// Zombie pointer
Zombie* createHorde(int numberOfZombies, IntRect arena);
