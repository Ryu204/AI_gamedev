// The gun intercepts mouse's future position and fire accordingly 
// ESC to escape

#include <iostream>
#include <vector>
#include <set>
#include <memory>
#include <cmath>
#include <queue>

#include <SFML/Graphics.hpp>

const float BULLET_RADIUS = 10.f;

template<typename T>
inline void center(T& ob)
{
	sf::FloatRect bounds = ob.getLocalBounds();
	ob.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
}

template<typename T>
float lengthOf(sf::Vector2<T> vec)
{
	return std::sqrt(vec.x * vec.x + vec.y * vec.y);
}

template<typename T>
sf::Vector2<T> normalise(sf::Vector2<T> vec)
{
	float mag = lengthOf<T>(vec);
	if (mag == 0)
		return sf::Vector2<T>();
	else
		return vec / mag;
}

template<typename T>
T lerp(T start, T end, float t)
{
	return start + (end - start) * t;
}

class Bullet : public sf::Drawable, public sf::Transformable
{
public:
	typedef std::unique_ptr<Bullet> Ptr;
public:
	Bullet(sf::Vector2f pos = sf::Vector2f(), sf::Vector2f vel = sf::Vector2f())
		: m_velocity(vel)
		, m_body(BULLET_RADIUS, 7)
	{
		sf::Transformable::setPosition(pos);
		center(m_body);
	};

	void update(sf::Time dt)
	{
		sf::Transformable::move(m_velocity * dt.asSeconds());
	}

	sf::FloatRect getBounds() const
	{
		return getTransform().transformRect(m_body.getGlobalBounds());
	};

	void setVelocity(sf::Vector2f vel)
	{
		m_velocity = vel;
	}

private:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override
	{
		states.transform *= getTransform();
		target.draw(m_body, states);
	};
private:
	sf::Vector2f m_velocity;
	sf::CircleShape m_body;
};

class BulletManager
{
public:
	BulletManager(sf::RenderWindow* win, int size)
		: m_window(win)
		, m_active(size)
	{
		for (int i = 0; i < size; i++)
		{
			m_IDs.push(i);
			m_bullets.push_back(Bullet::Ptr(std::make_unique<Bullet>()));
		}
	};

	void update(sf::Time dt)
	{
		sf::FloatRect screen = sf::FloatRect(sf::Vector2f(), sf::Vector2f(m_window->getSize()));
		std::vector<int> mark;
		for (int i = 0; i < m_bullets.size(); i++)
			if (m_active[i])
			{
				m_bullets[i]->update(dt);
				sf::FloatRect bounds = m_bullets[i]->getBounds();
				if (!bounds.intersects(screen))
					mark.push_back(i);
			}
		for (auto& i : mark)
			erase(i);
	};

	void add(sf::Vector2f pos, sf::Vector2f vel)
	{
		if (m_IDs.empty())
			return;
		int new_ID = m_IDs.front();
		m_IDs.pop();
		m_active[new_ID] = true;
		m_bullets[new_ID]->setPosition(pos);
		m_bullets[new_ID]->setVelocity(vel);
	};

	void render()
	{
		for (int i = 0; i < m_bullets.size(); i++)
			if (m_active[i])
				m_window->draw(*m_bullets[i]);
	}
private:
	void erase(int id)
	{
		m_IDs.push(id);
		m_active[id] = false;
	};
private:
	sf::RenderWindow* m_window;
	std::queue<int> m_IDs;
	std::vector<bool> m_active;
	std::vector<Bullet::Ptr> m_bullets;
};

class Shooter
{
public:
	Shooter(sf::RenderWindow* win, sf::Time interval, int bullet_count, float bullet_speed)
		: m_window(win)
		, m_interval(interval)
		, m_body(20, 10)
		, m_cursor(50, 20)
		, m_ray(sf::Lines, 2)
		, m_bullets(win, bullet_count)
		, m_speed(bullet_speed)
	{
		m_prev_pos = sf::Vector2f(sf::Mouse::getPosition(*m_window));
		m_new_pos = m_prev_pos;
		m_predict = m_prev_pos;
		center(m_body);
		center(m_cursor);
		m_body.setPosition(500.f, 500.f);
		m_cursor.setPosition(m_predict);
		m_cursor.setFillColor(sf::Color(0x99333399));
		m_ray[0].color = sf::Color::Green;
		m_ray[1].color = sf::Color::Green;
	};

	void update(sf::Time dt)
	{
		m_bullets.update(dt);
		m_current_time += dt;
		m_prev_pos = m_new_pos;
		m_new_pos = sf::Vector2f(sf::Mouse::getPosition(*m_window));
		sf::Time time_to_meet = std::min(sf::seconds(0.3f), sf::seconds(lengthOf(m_predict - m_body.getPosition()) / m_speed));
		sf::Vector2f mouse_velo = (m_new_pos - m_prev_pos) / dt.asSeconds();
		m_predict = m_new_pos + mouse_velo * time_to_meet.asSeconds();

		m_cursor.setPosition(lerp(m_cursor.getPosition(), m_predict, 0.1f));
		m_ray[0].position = m_body.getPosition();
		m_ray[1].position = m_ray[0].position + (m_cursor.getPosition() - m_ray[0].position) * 1.3f;

		if (m_current_time >= m_interval)
		{
			shoot(dt);
			m_current_time -= m_interval;
		}
	}

	void render()
	{
		m_window->draw(m_cursor);
		m_window->draw(m_ray);
		m_window->draw(m_body);
		m_bullets.render();
	}
private:
	void shoot(sf::Time dt)
	{
		m_bullets.add(m_body.getPosition(), m_speed * normalise(m_predict - m_body.getPosition()));
	};
private:
	sf::RenderWindow* m_window;
	const sf::Time m_interval;
	sf::Time m_current_time;
	sf::Vector2f m_prev_pos;
	sf::Vector2f m_new_pos;
	sf::Vector2f m_predict;
	BulletManager m_bullets;
	float m_speed;

	sf::CircleShape m_body;
	sf::CircleShape m_cursor;
	sf::VertexArray m_ray;
};

int main()
{
	sf::RenderWindow win(sf::VideoMode(1000, 1000), "HI", sf::Style::None);
	sf::Clock clock;
	sf::Time elapsed = clock.restart();
	sf::Time TPF = sf::seconds(1.f / 60);

	Shooter bao(&win, sf::seconds(0.2f), 20, 500);

	while (win.isOpen())
	{
		elapsed += clock.restart();
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
		bao.render();
		win.display();
	}
	return 0;
}