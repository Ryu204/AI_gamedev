#include "Boid.hpp"
#include "Utilise.hpp"

#include <iostream>

Entity::Entity(float acceleration, float rotate_speed)
	: m_state(0)
	, thrust_ratio(1.f)
	, rotate_ratio_left(1.f)
	, rotate_ratio_right(1.f)
	, m_acceleration(acceleration)
	, m_rotate_speed(rotate_speed)
	, m_speed(0)
{ }

sf::Vector2f Entity::globalToLocal(sf::Vector2f point) const
{
	float rot = Utilise::toRadian(getRotation());
	return sf::Vector2f(
		point.x * std::cos(rot) + point.y * std::sin(rot),
		point.y * std::cos(rot) - point.x * std::sin(rot));
}

sf::Vector2f Entity::localToGlobal(sf::Vector2f point) const
{
	float rot = Utilise::toRadian(-1.f * getRotation());
	return sf::Vector2f(
		point.x * std::cos(rot) + point.y * std::sin(rot),
		point.y * std::cos(rot) - point.x * std::sin(rot));
}

sf::Vector2f Entity::getVelocity() const
{
	return localToGlobal(sf::Vector2f(0.f, m_speed));
}

void Entity::update(sf::Time dt)
{
	if (m_state & RIGHT)
		rotate(rotate_ratio_right * m_rotate_speed * dt.asSeconds());
	if (m_state & LEFT)
		rotate(-rotate_ratio_left * m_rotate_speed * dt.asSeconds());
	if (m_state & THRUST)
		m_speed += m_acceleration * thrust_ratio * dt.asSeconds();
	m_speed -= DRAG_CONST * m_speed * m_speed * dt.asSeconds();
	m_speed = std::max(0.f, m_speed);
	move(getVelocity() * dt.asSeconds());
}

Boid::Boid(float acceleration, float rotate_speed, 
	float radius, float angle, float feeler_length, float feeler_angle)
	: Entity(acceleration, rotate_speed)
	, m_view_radius(radius)
	, m_view_angle(angle)
	, m_feeler_length(feeler_length)
	, m_feeler_angle(feeler_angle)
	, m_body(sf::Triangles, 3)
{ 
	m_body[0].position = sf::Vector2f(0, 5);
	m_body[1].position = sf::Vector2f(-2.5f, -2);
	m_body[2].position = sf::Vector2f(2.5f, - 2);

	sf::Color c(rand() % 256, rand() % 256, rand() % 256);
	for (int i = 0; i < 3; i++)
		m_body[i].color = c;
}

void Boid::setThruster(bool enable, float ratio)
{
	ratio = std::min(1.f, std::max(ratio, 0.f));
	thrust_ratio = ratio;
	if (enable)
		m_state |= THRUST;
	else
		m_state &= ~THRUST;
}

void Boid::turn(bool left, float ratio)
{
	if (left)
	{
		rotate_ratio_left += ratio;
		m_state |= LEFT;
	}
	else
	{
		rotate_ratio_right += ratio;
		m_state |= RIGHT;
	}
}

void Boid::turnOff()
{
	rotate_ratio_left = 0.f;
	m_state &= ~LEFT;
	rotate_ratio_right = 0.f;
	m_state &= ~RIGHT;
}

void Boid::updateData(const std::vector<Boid*>& boids)
{
	float left = 0.f, right = 0.f;
	std::vector<Boid*> visible_units;
	for (Boid* i : boids)
	{
		sf::Vector2f dis = globalToLocal(i->getPosition() - getPosition());
		float lth = Utilise::lengthOf(dis);
		if (lth <= m_view_radius && i != this)
		{
			if (m_view_angle <= 90 && dis.y >= 0)
			{
				if (abs(dis.x) <= m_view_radius * std::sin(Utilise::toRadian(m_view_angle)))
					visible_units.push_back(i);
			}

			else if (m_view_angle > 90)
			{
				if (dis.y >= 0)
					visible_units.push_back(i);
				else if (abs(dis.y) <= m_view_radius * std::cos(Utilise::toRadian(180 - m_view_angle)))
					visible_units.push_back(i);
			}
		}
	}
	// Separation
	sf::Vector2f steer;
	for (Boid* i : visible_units)
	{
		sf::Vector2f dis = i->getPosition() - getPosition();
		float lth = Utilise::lengthOf(dis);
		if (2.5 * lth <= m_view_radius)
		{
			dis = globalToLocal(dis);
			if (dis.x > 0)
				turn(false, 0.2f * m_view_radius / (2.5f * lth));
			else
				turn(true, 0.2f * m_view_radius / (2.5f * lth));
		}
	}
	// Alignment
	sf::Vector2f vel;
	for (Boid* i : visible_units)
		vel += i->getVelocity();
	if (visible_units.size())
	{
		vel = vel / (1.f * visible_units.size());
		sf::Vector2f dir_norm = Utilise::normalise(getVelocity());
		vel = Utilise::normalise(vel);
		float prod = dir_norm.x * vel.x + dir_norm.y * vel.y;
		float frac = std::acos(std::min(1.f, std::max(-1.f, prod))) / Utilise::PI;
		if (globalToLocal(vel).x < 0)
			turn(false, frac);
		else
			turn(true, frac);
	}

	// Cohesion
	sf::Vector2f pos;
	for (Boid* i : visible_units)
		pos += i->getPosition();
	if (visible_units.size())
	{
		pos = pos / (1.f * visible_units.size());
		sf::Vector2f dir_norm = Utilise::normalise(getVelocity());
		sf::Vector2f dis_norm = Utilise::normalise(pos - getPosition());
		float prod = dir_norm.x * dis_norm.x + dir_norm.y * dis_norm.y;
		float frac = std::acos(std::min(1.f, std::max(-1.f, prod))) / Utilise::PI;
		if (globalToLocal(dis_norm).x < 0)
			turn(false, frac);
		else
			turn(true, frac);
	}
}

void Boid::updateFeeler(const std::vector<std::unique_ptr<Obstacle>>& obstacles)
{
	float left = 2.f, right = 2.f;
	float angle = Utilise::toRadian(m_feeler_angle);
	sf::Vector2f antenna(m_feeler_length * std::sin(angle), m_feeler_length * std::cos(angle));
	sf::Vector2f left_feeler = localToGlobal(antenna);
	for (int i = 0; i < obstacles.size(); i++)
	{
		bool collide = false;
		float time = obstacles[i]->checkRay(getPosition(), getPosition() + left_feeler, collide);
		if (collide)
			left = std::min(left, time);
	}
	antenna.x *= -1.f;
	sf::Vector2f right_feeler = localToGlobal(antenna);
	for (int i = 0; i < obstacles.size(); i++)
	{
		bool collide = false;
		float time = obstacles[i]->checkRay(getPosition(), getPosition() + right_feeler, collide);
		if (collide)
			right = std::min(right, time);
	}
	if (left < right && left < 1.f)
	{
		turn(false, 1 / left);
		setThruster(true, left);
	}
	else if (right < left && right < 1.f)
	{
		turn(true, 1 / right);
		setThruster(true, right);
	}
}

void Boid::batchVertices(sf::VertexArray& arr) const
{
	sf::Transform trans = getTransform();
	for (int i = 0; i < 3; i++)
	{
		sf::Vertex ver = m_body[i];
		ver.position = trans.transformPoint(ver.position);
		arr.append(ver);
	}
}