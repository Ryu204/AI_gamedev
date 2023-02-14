#include <cmath>
#include <deque>
#include <vector>
#include <iostream>

#include <SFML/Graphics.hpp>

#include "Utilise.hpp"
#include "PManager.hpp"
#include "Rider.hpp"

const float SCREEN_SIZE = 1000.f;

struct Movement
{
	float length = 100.f;
	float angle = 90.f;
	bool will_go = true;
};

const std::vector<Movement> SQUARE = {
	Movement{ 200.f },
	Movement{ 0.f, 90.f, false },
};

const std::vector<Movement> CIRCLE = {
	Movement{ 10.f },
	Movement{ 0.f, 5.f, false },
};

const std::vector<Movement> ZIGZAG = {
	Movement{ 100.f },
	Movement{ 0.f, 70.f, false },
	Movement{ 100.f },
	Movement{ 0.f, -70.f, false },
};

class Guard : public Rider
{
public:
	Guard(float jet_strength, float steer_force)
		: Rider(jet_strength, steer_force)
		, m_done(true)
	{
		m_patterns.registerType<Movement>("Move");
		for (const auto& i : ZIGZAG)
			m_patterns.push<Movement>(i);
	}

	void update(sf::Time dt)
	{
		if (m_is_going && Utilise::lengthOf(m_initial_position - getPosition()) >= m_goal_length)
			m_done = true;
		if (m_is_going == false)
		{
			float change = abs(direction - m_initial_angle);
			if (change >= abs(m_goal_rotation))
				m_done = true;
			else
			{
				steer_ratio = std::min(1.f, 1.f - change / abs(m_goal_rotation));
				if (steer_ratio < 0.05f)
				{
					steer_ratio = 0.05f;
				}
			}
		}
		if (m_done)
			takeCommand(m_patterns.next<Movement>());
		Rider::update(dt);
	}
private:
	// angle is in degree, result is in [0, 360)
	float modulusAngle(float angle)
	{
		float q = std::floor(angle / 360.f);
		return angle - q * 360;
	}

	void takeCommand(const Movement& val)
	{
		m_done = false;
		if (val.will_go)
		{
			m_is_going = true;
			m_initial_position = getPosition();
			m_goal_length = val.length;
			thruster = true;
			steer = NONE;
		}
		else
		{
			m_is_going = false;
			m_initial_angle = direction;
			m_goal_rotation = val.angle;
			thruster = false;
			steer = val.angle < 0.f ? LEFT : RIGHT;
			steer_ratio = 1.f;
		}
	}
private:
	bool m_done;
	bool m_is_going;
	PatternManager m_patterns;

	sf::Vector2f m_initial_position;
	float m_initial_angle;
	float m_goal_length;
	float m_goal_rotation;
};

int main()
{
	sf::RenderWindow win(sf::VideoMode(1000, 1000), "HI", sf::Style::None);
	sf::Clock clock;
	sf::Time elapsed = clock.restart();
	sf::Time total = elapsed;
	sf::Time TPF = sf::seconds(1.f / 60);

	Guard bao(300, 300);
	bao.setPosition(0, 0);
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
		}
		while (elapsed >= TPF)
		{
			elapsed -= TPF;
			bao.update(TPF);
		}
		win.clear();
		win.draw(bao);
		win.display();
	}
	return 0;
}