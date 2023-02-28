#ifndef AI_FLOCK_OBS
#define AI_FLOCK_OBS

#include <memory>

#include <SFML/Graphics.hpp>

class Obstacle : public sf::Drawable
{
public:
	Obstacle();
	virtual ~Obstacle() = default;

	virtual float checkRay(sf::Vector2f start, sf::Vector2f end, bool& collide) const = 0;

	virtual sf::FloatRect getBounds() const = 0;
};

class Rectangle : public Obstacle
{
public:
	Rectangle(sf::FloatRect bound);

	float checkRay(sf::Vector2f start, sf::Vector2f end, bool& collide) const override;

	sf::FloatRect getBounds() const override;
private:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
private:
	sf::RectangleShape m_body;
};

class Circle : public Obstacle
{
public:
	Circle(float radius, sf::Vector2f position);

	float checkRay(sf::Vector2f start, sf::Vector2f end, bool& collide) const override;

	sf::FloatRect getBounds() const override;
private:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
private:
	sf::CircleShape m_body;
};

#endif 