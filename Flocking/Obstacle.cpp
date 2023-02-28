#include "Obstacle.hpp"
#include "Utilise.hpp"

#include <numeric>

Obstacle::Obstacle()
{  }

Rectangle::Rectangle(sf::FloatRect bound)
	: m_body(sf::Vector2f(bound.width, bound.height))
{
	m_body.setPosition(bound.left, bound.top);
	m_body.setFillColor(sf::Color(rand() % 256, rand() % 256, rand() % 256));
	m_body.setOutlineColor(sf::Color::Green);
	m_body.setOutlineThickness(1.f);
}

float Rectangle::checkRay(sf::Vector2f start, sf::Vector2f end, bool& collide) const
{
	sf::FloatRect ray_bounds(std::min(start.x, end.x), std::min(start.y, end.y), abs(start.x - end.x), abs(start.y - end.y));
	sf::FloatRect bounds = getBounds();
	if (!ray_bounds.intersects(bounds))
	{
		collide = false;
		return 0.f;
	}
	sf::Vector2f near, far;
	if (start.x == end.x)
	{
		near.x = -std::numeric_limits<float>::infinity();
		far.x = std::numeric_limits<float>::infinity();
	}
	else
	{
		near.x = (bounds.left + bounds.width - start.x) / (end.x - start.x);
		far.x = (bounds.left - start.x) / (end.x - start.x);
		if (near.x > far.x)
			std::swap(near.x, far.x);
	}
	if (start.y == end.y)
	{
		near.y = -std::numeric_limits<float>::infinity();
		far.y = std::numeric_limits<float>::infinity();
	}
	else
	{
		near.y = (bounds.top + bounds.height - start.y) / (end.y - start.y);
		far.y = (bounds.top - start.y) / (end.y - start.y);
		if (near.y > far.y)
			std::swap(near.y, far.y);
	}
	float entry = std::max(near.x, near.y), exit = std::min(far.x, far.y);
	if (entry > exit || far.x < 0.f || far.y < 0.f || near.x > 1.f || near.y > 1.f)
	{
		collide = false;
		return 0.f;
	}
	collide = true;
	return entry;
}

sf::FloatRect Rectangle::getBounds() const
{
	return m_body.getGlobalBounds();
}

void Rectangle::draw(sf::RenderTarget& target, sf::RenderStates state) const
{
	target.draw(m_body, state);
}

Circle::Circle(float radius, sf::Vector2f position)
	: m_body(radius)
{
	m_body.setFillColor(sf::Color(rand() % 256, rand() % 256, rand() % 256));
	m_body.setOutlineColor(sf::Color::Green);
	m_body.setOutlineThickness(1.f);
	Utilise::center(m_body);
	m_body.setPosition(position);
}

float Circle::checkRay(sf::Vector2f start, sf::Vector2f end, bool& collide) const
{
	sf::Vector2f center(m_body.getPosition());
	float radius(m_body.getRadius());
	sf::Vector2f vel = end - start, to_center = center - start;
	float vel_length = Utilise::lengthOf(vel), to_center_len = Utilise::lengthOf(to_center);
	float length = Utilise::product(vel, to_center) / vel_length;
	if (length < 0)
	{
		collide = false;
		return 0;
	}
	float time = (length + 0.5f - std::sqrt(length * length + radius * radius - to_center_len * to_center_len)) / vel_length;
	float dist = std::sqrt(to_center_len * to_center_len - length * length);
	if (time <= 1.f && dist < radius)
	{
		collide = true;
		return time;
	}
	else
	{
		collide = false;
		return 0.f;
	}
}

sf::FloatRect Circle:: getBounds() const
{
	return m_body.getGlobalBounds();
}

void Circle::draw(sf::RenderTarget& target, sf::RenderStates state) const
{
	target.draw(m_body, state);
}