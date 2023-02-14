#include "Rider.hpp"
#include "Utilise.hpp"

#include <cmath>

Entity::Entity(float jet_strength, float steer_force)
	: steer_ratio(1.f)
	, push_acceleration(jet_strength)
	, steer(NONE)
	, thruster(false)
	, direction(-90)
	, m_steer_value(steer_force)
	, m_push_speed(0)
	, m_drag_constant(0.7f)
{
	setPosition(500.f, 500.f);
	setRotation(-90);
}

void Entity::update(sf::Time dt)
{
	if (steer == LEFT)
	{
		rotate(-abs(m_steer_value) * steer_ratio * dt.asSeconds());
		direction += -abs(m_steer_value) * steer_ratio * dt.asSeconds();
	}
	else if (steer == RIGHT)
	{
		rotate(abs(m_steer_value) * steer_ratio * dt.asSeconds());
		direction += -abs(m_steer_value) * steer_ratio * dt.asSeconds();
	}
	if (thruster)
		m_push_speed += push_acceleration * dt.asSeconds();
	m_push_speed += (-m_drag_constant * m_push_speed) * dt.asSeconds();
	m_push_speed = std::max(0.f, m_push_speed);
	sf::Vector2f velocity(m_push_speed * std::sin(Utilise::toRadian(-getRotation())),
		m_push_speed * std::cos(Utilise::toRadian(-getRotation())));
	move(velocity * dt.asSeconds());
}

sf::Vector2f Entity::getVelocity()
{
	return sf::Vector2f(m_push_speed * std::sin(Utilise::toRadian(-getRotation())),
		m_push_speed * std::cos(Utilise::toRadian(-getRotation())));
}

Rider::Rider(float jet_strength, float steer_force)
	: Entity(jet_strength, steer_force)
	, m_body(sf::Vector2f(10, 20))
	, m_image_count(20)
	, m_image_interval(sf::seconds(0.05f))
{
	sf::Vector2f size = m_body.getSize();
	size /= 2.f;
	m_body.setOrigin(size);
	for (int i = 0; i < m_image_count; i++)
		m_previous_transforms.push_back(getTransform());
	m_body.setFillColor(sf::Color::Green);
}

void Rider::update(sf::Time dt)
{
	Entity::update(dt);

	m_elapsed_time += dt;
	while (m_elapsed_time >= m_image_interval)
	{
		m_elapsed_time -= m_image_interval;
		m_previous_transforms.pop_front();
		m_previous_transforms.push_back(getTransform());
	}
}

sf::FloatRect Rider::getBounds()
{
	return getTransform().transformRect(m_body.getGlobalBounds());
}

void Rider::draw(sf::RenderTarget& target, sf::RenderStates states) const 
{
	sf::Color color = m_body.getFillColor();
	color.a = 20;
	m_body.setScale(sf::Vector2f());
	for (auto& i : m_previous_transforms)
	{
		auto st = states;
		st.transform *= i;
		color.a = (color.a + 255) / 3;
		m_body.setFillColor(color);
		m_body.setScale((m_body.getScale() + sf::Vector2f(1.f, 1.f)) / 2.5f);
		target.draw(m_body, st);
	}
	color.a = 255;
	m_body.setScale(sf::Vector2f(1.f, 1.f));
	m_body.setFillColor(color);
	states.transform *= getTransform();
	target.draw(m_body, states);
}
	
sf::Vector2f Rider::translate(sf::Vector2f point)
{
	float rot = getRotation();
	return sf::Vector2f(
		point.x * std::cos(Utilise::toRadian(rot)) + point.y * std::sin(Utilise::toRadian(rot)),
		-point.x * std::sin(Utilise::toRadian(rot)) + point.y * std::cos(Utilise::toRadian(rot)));
}