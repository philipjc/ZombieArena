#include "bullet.h"

// The constructor
Bullet::Bullet()
{
	m_BulletShape.setSize(sf::Vector2f(2, 2));
}

void Bullet::shoot(float startX, float startY, float targetX, float targetY)
{
	// Keep track of the bullet
	m_InFlight = true;
	m_Position.x = startX;
	m_Position.y = startY;

	/*
		Now, we use a bit of trigonometry to determine the gradient of travel for a bullet. 
		The progression of the bullet, both horizontally and vertically, must vary based on 
		the slope of the line that's created by drawing between the start and target of a bullet. 
		The rate of change cannot be the same or very steep shots will arrive at the horizontal 
		location before the vertical location, and vice versa for shallow shots.

		The code that follows derives the gradient based on the equation of a line. 
		Then, it checks whether the gradient is less than zero and if it is, multiplies it by -1. This 
		is because the start and target coordinates that are passed in can be negative or 
		positive, and we always want the amount of progression each frame to be positive.

		Multiplying by -1 simply makes the negative number into its positive equivalent 
		because a minus multiplied by a minus gives a positive. The actual direction of travel 
		will be handled in the update function by adding or subtracting the positive values 
		we arrive at in this function.
	*/

	// Calculate the gradient of the flight path
	float gradient = (startX - targetX) / (startY - targetY);

	// Any gradient less than 1 needs to be negative
	if (gradient < 0)
	{
		gradient *= -1;
	}

	// Calculate the ratio between x and y
	float ratioXY = m_BulletSpeed / (1 + gradient);

	// Set the "speed" horizontally and vertically
	m_BulletDistanceY = ratioXY;
	m_BulletDistanceX = ratioXY * gradient;

	// Point the bullet in the right direction
	if (targetX < startX)
	{
		m_BulletDistanceX *= -1;
	}

	if (targetY < startY)
	{
		m_BulletDistanceY *= -1;
	}

	// Set a max range of 1000 pixels
	float range = 1000;

	m_MinX = startX - range;
	m_MaxX = startX + range;
	m_MinY = startY - range;
	m_MaxY = startY + range;

	// Position the bullet ready to be drawn
	m_BulletShape.setPosition(m_Position);
}

void Bullet::stop()
{
	m_InFlight = false;
}

bool Bullet::isInFlight()
{
	return m_InFlight;
}

FloatRect Bullet::getPosition()
{
	return m_BulletShape.getGlobalBounds();
}

RectangleShape Bullet::getShape()
{
	return m_BulletShape;
}

void Bullet::update(float elapsedTime)
{
	// Update the bullet position variables
	m_Position.x += m_BulletDistanceX * elapsedTime;
	m_Position.y += m_BulletDistanceY * elapsedTime;

	// Move the bullet
	m_BulletShape.setPosition(m_Position);

	// Create readable max variables
	float reachedMinX = m_Position.x < m_MinX;
	float reachedMaxX = m_Position.x > m_MaxX;
	
	float reachedMinY = m_Position.y < m_MinY;
	float reachedMaxY = m_Position.y > m_MaxY;

	// Has the bullet gone out of range?
	if (reachedMinX || reachedMaxX || reachedMinY || reachedMaxY)
	{
		m_InFlight = false;
	}
}
