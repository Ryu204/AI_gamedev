#include <cmath>
#include <deque>
#include <iostream>

#include <SFML/Graphics.hpp>

#include "Utilise.hpp"

const float SCREEN_SIZE = 1000.f;

class Entity : public sf::Transformable
{
public:
	enum SideSteer
	{
		LEFT, RIGHT, NONE
	};
public:
	Entity(float steer_force)
		: push_acceleration(0.f)
		, steer(NONE)
		, m_steer_value(steer_force)
		, m_push_speed(0)
		, m_drag_constant(0.7f)
	{	
		setPosition(SCREEN_SIZE / 2.f, SCREEN_SIZE / 2.f);
		setRotation(180);
	}

	void update(sf::Time dt)
	{
		if (steer == LEFT)
			rotate(-abs(m_steer_value) * dt.asSeconds());
		if (steer == RIGHT)
			rotate(abs(m_steer_value) * dt.asSeconds());
		m_push_speed += push_acceleration * dt.asSeconds();
		m_push_speed += (-m_drag_constant * m_push_speed) * dt.asSeconds();
		m_push_speed = std::max(0.f, m_push_speed);
		sf::Vector2f velocity(m_push_speed * std::sin(Utilise::toRadian(-getRotation())),
							  m_push_speed * std::cos(Utilise::toRadian(-getRotation())));
		move(velocity * dt.asSeconds());
	}

	sf::Vector2f getVelocity()
	{
		return sf::Vector2f(m_push_speed * std::sin(Utilise::toRadian(-getRotation())),
							m_push_speed * std::cos(Utilise::toRadian(-getRotation())));
	}
public:
	// The acceleration ~ force applied to the back
	float push_acceleration;
	SideSteer steer;
private:
	// Unit: degree/s
	float m_steer_value;
	float m_push_speed;
	float m_drag_constant;
};

class Rider : public Entity, public sf::Drawable
{
public:
	Rider(float jet_strength, float steer_force)
		: Entity(steer_force)
		, m_body(sf::Vector2f(10, 20))
		, m_straight_acceleration(jet_strength)
		, m_thrust(false)
		, m_image_count(20)
		, m_image_interval(sf::seconds(0.05f))
	{
		// Utilise::center(m_body);
		sf::Vector2f size = m_body.getSize();
		size.x /= 2;
		m_body.setOrigin(size);
		for (int i = 0; i < m_image_count; i++)
			m_previous_transforms.push_back(getTransform());
		m_body.setFillColor(sf::Color::Green);
	}

	void processInput(sf::Time dt)
	{
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
			push_acceleration = m_straight_acceleration;
		else
			push_acceleration = 0.f;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
			steer = Entity::LEFT;
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
			steer = Entity::RIGHT;
		else
			steer = Entity::NONE;
	}

	virtual void update(sf::Time dt)
	{
		Entity::update(dt);
		sf::Vector2f pos = getPosition();
		pos.x = std::max(std::min(pos.x, SCREEN_SIZE - 10.f), 10.f);
		pos.y = std::max(std::min(pos.y, SCREEN_SIZE - 10.f), 10.f);
		setPosition(pos);

		m_elapsed_time += dt;
		while (m_elapsed_time >= m_image_interval)
		{
			m_elapsed_time -= m_image_interval;
			m_previous_transforms.pop_front();
			m_previous_transforms.push_back(getTransform());
		}
	}

	sf::FloatRect getBounds()
	{
		return getTransform().transformRect(m_body.getGlobalBounds());
	}
private:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override
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
protected:
	mutable sf::RectangleShape m_body;
private:
	float m_straight_acceleration;
	bool m_thrust;

	const int m_image_count;
	std::deque<sf::Transform> m_previous_transforms;
	sf::Time m_image_interval;
	sf::Time m_elapsed_time;
};

class Chaser : public Rider
{
public:
	enum Type
	{
		DIRECT, INTERCEPT
	};
public:
	Chaser(Rider* prey, float jet_strength, float steer_strength)
		: Rider(0, steer_strength)
		, m_prey(prey)
		, m_type(DIRECT)
		, m_cursor(10, 8)
		, m_show_cursor(false)
	{ 
		m_body.setFillColor(sf::Color::Red);
		push_acceleration = jet_strength;
		m_cursor.setFillColor(sf::Color::Red);
		Utilise::center(m_cursor);
		std::cout << "Press Enter to switch chasing mode\n";
	}

	void processInput(const sf::Event& e)
	{
		if (e.type == sf::Event::KeyPressed)
		{
			if (e.key.code == sf::Keyboard::Enter)
			{
				std::cout << "Switched mode. Current mode is ";
				if (m_type == DIRECT)
				{
					m_type = INTERCEPT;
					std::cout << "INTERCEPT\nPress Spacebar to display prediction\n";
				}
				else
				{
					m_type = DIRECT;
					std::cout << "DIRECT CHASING\n";
				}
			}
			else if (e.key.code == sf::Keyboard::Space)
				m_show_cursor = !m_show_cursor;
		}
	}

	void update(sf::Time dt) override
	{
		sf::Vector2f dis;
		if (m_type == DIRECT)
			dis = m_prey->getPosition() - getPosition();
		else
		{
			float rela_speed = Utilise::lengthOf(getVelocity() - m_prey->getVelocity());
			float rela_dis = Utilise::lengthOf(getPosition() - m_prey->getPosition());
			rela_speed += 1.f;
			float time_to_intercept = rela_dis / rela_speed;
			m_intercept_point = m_prey->getPosition() + m_prey->getVelocity() * time_to_intercept;
			dis = m_intercept_point - getPosition();
			m_cursor.setPosition(m_intercept_point);
		}
		dis = Utilise::normalise(translate(dis));
		if (dis.x >= 0.1f)
			steer = Entity::LEFT;
		else if (dis.x <= -0.1f)
			steer = Entity::RIGHT;
		else if (dis.y < 0)
			steer = Entity::LEFT;
		else
			steer = Entity::NONE;
		Rider::update(dt);
	}
private:
	sf::Vector2f translate(sf::Vector2f point)
	{
		float rot = getRotation();
		return sf::Vector2f(
			point.x * std::cos(Utilise::toRadian(rot)) + point.y * std::sin(Utilise::toRadian(rot)),
			-point.x * std::sin(Utilise::toRadian(rot)) + point.y * std::cos(Utilise::toRadian(rot)));
	}
	
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override
	{
		if (m_type == INTERCEPT && m_show_cursor)
			target.draw(m_cursor, states);
		target.draw(static_cast<Rider>(*this), states);
	}
private:
	Rider* m_prey;
	Type m_type;
	sf::Vector2f m_intercept_point;
	sf::CircleShape m_cursor;
	bool m_show_cursor;
};

int main()
{
	sf::RenderWindow win(sf::VideoMode(1000, 1000), "HI", sf::Style::None);
	sf::Clock clock;
	sf::Time elapsed = clock.restart();
	sf::Time total = elapsed;
	sf::Time TPF = sf::seconds(1.f / 60);

	Rider bao(300, 350);
	bao.setPosition(300, 300);
	Chaser killer(&bao, 150,350);
	killer.setPosition(sf::Vector2f());

	while (win.isOpen())
	{
		sf::Time dt = clock.restart();
		elapsed += dt;
		total += dt;
		sf::Event e;
		while (win.pollEvent(e))
		{
			if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Escape)
				win.close();
			killer.processInput(e);
		}
		while (elapsed >= TPF)
		{
			elapsed -= TPF;
			bao.processInput(TPF);
			bao.update(TPF);
			killer.update(TPF);
		}
		win.clear();
		win.draw(bao);
		win.draw(killer);
		win.display();
	}
	return 0;
}