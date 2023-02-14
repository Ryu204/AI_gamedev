#ifndef AI_PTPS_RIDER
#define AI_PTPS_RIDER

#include <deque>
#include <SFML/Graphics.hpp>

class Entity : public sf::Transformable
{
public:
	enum SideSteer
	{
		LEFT, RIGHT, NONE
	};
public:
	Entity(float jet_strength, float steer_force);
	void update(sf::Time dt);
	sf::Vector2f getVelocity();
public:
	// The acceleration ~ force applied to the back
	// max is 1
	float steer_ratio;
	float push_acceleration;
	SideSteer steer;
	bool thruster;
	// Value of getRotation but unnormalised
	float direction;
private:
	// Unit: degree/s
	float m_steer_value;
	float m_push_speed;
	float m_drag_constant;
};

class Rider : public Entity, public sf::Drawable
{
public:
	Rider(float jet_strength, float steer_force);

	void update(sf::Time dt);

	sf::FloatRect getBounds();
private:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	sf::Vector2f translate(sf::Vector2f point);
protected:
	mutable sf::RectangleShape m_body;
private:
	const int m_image_count;
	std::deque<sf::Transform> m_previous_transforms;
	sf::Time m_image_interval;
	sf::Time m_elapsed_time;
};

#endif