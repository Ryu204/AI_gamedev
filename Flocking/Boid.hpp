#ifndef AI_FLOCK_BOID
#define AI_FLOCK_BOID

#include <vector>

#include <SFML/Graphics.hpp>

#include "Obstacle.hpp"

const float DRAG_CONST = 0.05f;

class Entity : public sf::Transformable
{
public:
	enum State
	{
		NONE = 0,
		RIGHT = 1 << 0,
		LEFT = 1 << 1,
		THRUST = 1 << 2
	};
public:
	Entity(float acceleration = 1000, float rotate_speed = 200);
	
	sf::Vector2f globalToLocal(sf::Vector2f point) const;

	sf::Vector2f localToGlobal(sf::Vector2f point) const;

	sf::Vector2f getVelocity() const;

	void update(sf::Time dt);
protected:
	int m_state;
	float thrust_ratio;
	float rotate_ratio_left;
	float rotate_ratio_right;
private:
	// Fixed variables
	float m_acceleration;
	float m_rotate_speed;
	// Changing variables
	float m_speed;
};

class Boid : public Entity
{
public:
	Boid(float acceleration = 1000, float rotate_speed = 200,
		float radius = 50, float angle = 90,
		float feeler_length = 100, float feeler_angle = 10);

	void setThruster(bool enable, float ratio);

	void turn(bool left, float ratio);
	
	void turnOff();

	void updateData(const std::vector<Boid*>& boids);

	void updateFeeler(const std::vector<std::unique_ptr<Obstacle>>& obstacles);

	void batchVertices(sf::VertexArray& arr) const;
private:
	float m_view_radius;
	float m_view_angle; // varies from 0 to 180 at max
	float m_feeler_length;
	float m_feeler_angle;
	sf::VertexArray m_body;
};

#endif