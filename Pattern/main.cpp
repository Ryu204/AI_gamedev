#include <iostream>
#include <cassert>
#include <SFML/Graphics.hpp>

#include "Pattern.hpp"
#include "Bersenham_line.hpp"

// path on an axis cannot be longer than 50
struct Movement
{
	sf::Vector2i path = sf::Vector2i();
	float speed = 10.f; // "pixel" / sec
};

struct Attack
{
	enum Type
	{
		RED, PURPLE, GREEN
	};
	Type type = RED;
	sf::Time time = sf::seconds(1.f);
};

struct Wait
{
	sf::Time time = sf::seconds(1.f);
};

class Guard
{
public:
	Guard()
		: coordinate(1, 1)
		, color(sf::Color::Red)
		, done(true)
		, m_state("")
		, m_current_index(0)
	{  };

	void takeCommand(const Movement& val)
	{
		color = sf::Color::Black;
		done = false;
		m_state = "Move";
		auto vec = bersenham_line(sf::Vector2i(), val.path);
		m_path.resize(vec.size() - 1);
		for (int i = vec.size() - 1; i >= 1; i--)
			m_path[i - 1] = vec[i] - vec[i - 1];
		m_current_index = 0;
		m_total = sf::seconds(1.f / val.speed);
		m_elapsed = sf::Time::Zero;
	}

	void takeCommand(const Attack& val)
	{
		if (val.type == Attack::GREEN)
			color = sf::Color::Green;
		else if (val.type == Attack::RED)
			color = sf::Color::Red;
		else
			color = sf::Color(255, 0, 255);
		done = false;
		m_state = "Attack";
		m_total = val.time;
		m_elapsed = sf::Time::Zero;
	}

	void takeCommand(const Wait& val)
	{
		color = sf::Color::Yellow;
		done = false;
		m_state = "Wait";
		m_total = val.time;
		m_elapsed = sf::Time::Zero;
	}

	void update(sf::Time dt)
	{
		if (done)
			return;
		m_elapsed += dt;
		if (m_state == "Move")
		{
			if (m_path.empty())
			{
				done = true;
			}
			else
				while (m_elapsed >= m_total)
				{
					m_elapsed -= m_total;
					coordinate.x += m_path[m_current_index].x;
					coordinate.y += m_path[m_current_index].y;
					m_current_index++;
					if (m_current_index >= m_path.size())
						done = true;
				}
		}
		else if (m_state == "Attack")
		{
			if (m_elapsed >= m_total)
				done = true;
		}
		else if (m_state == "Wait")
		{
			if (m_elapsed >= m_total)
				done = true;
		}
	}
public:
	sf::Vector2i coordinate;
	sf::Color color;
	bool done;
private:
	std::string m_state;
	std::vector<sf::Vector2i> m_path;
	unsigned int m_current_index;
	sf::Time m_total;
	sf::Time m_elapsed;
};

void init(PatternManager& pm)
{
	pm.registerType<Movement>("Move");
	pm.registerType<Attack>("Attack");
	pm.registerType<Wait>("Wait");

	pm.push(Attack{ Attack::GREEN, sf::seconds(1.5f) });
	pm.push(Attack{ Attack::PURPLE, sf::seconds(1.6f) });
	pm.push(Movement{ sf::Vector2i(5, 0), 5.f });
	pm.push(Movement{ sf::Vector2i(0, 5), 5.f });
	pm.push(Movement{ sf::Vector2i(-5, 0), 5.f });
	pm.push(Movement{ sf::Vector2i(0, -5), 5.f });
	pm.push(Movement{ sf::Vector2i(7, 7), 10.f });
	pm.push(Movement{ sf::Vector2i(-7, -7), 10.f });
}

void init2(PatternManager& pm)
{
	pm.registerType<Movement>("Move");

	pm.push(Movement{ sf::Vector2i(9,0), 5.f });
	pm.push(Movement{ sf::Vector2i(13, -2), 5.f });
	pm.push(Movement{ sf::Vector2i(3, 11), 5.f });
	pm.push(Movement{ sf::Vector2i(-3, 3), 5.f });
	pm.push(Movement{ sf::Vector2i(-7, -3), 10.f });
	pm.push(Movement{ sf::Vector2i(-4, 0), 10.f });
	pm.push(Movement{ sf::Vector2i(-8, 4), 10.f });
	pm.push(Movement{ sf::Vector2i(-3, -13), 10.f });
}

class Grid
{
public:
	Grid(sf::RenderWindow* win, unsigned int size)
		: m_window(win)
		, m_size(size)
		, m_cell_size(1000.f / size)
		, m_lines(sf::Lines)
		, m_patterns(2)
		, m_guard(2)
		, m_body(2, sf::RectangleShape(sf::Vector2f(m_cell_size, m_cell_size)))
	{
		assert(size > 1 && "Must have at least 4 cells");

		for (int i = 0; i <= size; i++)
		{
			m_lines.append(sf::Vertex{ sf::Vector2f(i * m_cell_size, 0) });
			m_lines.append(sf::Vertex{ sf::Vector2f(i * m_cell_size, 1000.f) });
			m_lines.append(sf::Vertex{ sf::Vector2f(0, i * m_cell_size) });
			m_lines.append(sf::Vertex{ sf::Vector2f(1000.f, i * m_cell_size) });
		}

		for (int i = 0; i < m_lines.getVertexCount(); i++)
			m_lines[i].color = sf::Color::Black;
		m_guard[0].coordinate = sf::Vector2i(50, 50);
		m_guard[1].coordinate = sf::Vector2i(50, 10);
		init2(m_patterns[0]);
		init(m_patterns[1]);
	};

	void update(sf::Time dt)
	{
		for (int i = 0; i < 2; i++)
		{
			m_guard[i].update(dt);
			if (m_guard[i].done)
			{
				std::string move = m_patterns[i].typeOfNext();
				if (move == "Move")
					m_guard[i].takeCommand(m_patterns[i].next<Movement>());
				if (move == "Attack")
					m_guard[i].takeCommand(m_patterns[i].next<Attack>());
				if (move == "Wait")
					m_guard[i].takeCommand(m_patterns[i].next<Wait>());
				m_guard[i].done = false;
			}
			m_body[i].setFillColor(m_guard[i].color);
			m_body[i].setPosition(m_cell_size * sf::Vector2f(m_guard[i].coordinate));
		}
	}

	void render()
	{
		for (auto& i : m_body)
		m_window->draw(i);
		m_window->draw(m_lines);
	}
private:
	sf::RenderWindow* m_window;
	unsigned int m_size;
	float m_cell_size;
	sf::VertexArray m_lines;

	std::vector<PatternManager> m_patterns;
	std::vector<Guard> m_guard;
	std::vector<sf::RectangleShape> m_body;
};

int main()
{
	sf::RenderWindow win(sf::VideoMode(1000, 1000), "HI", sf::Style::None);
	win.setFramerateLimit(100);
	sf::Clock clock;
	sf::Time elapsed = clock.restart();
	sf::Time TPF = sf::seconds(1.f / 60);

	Grid grid(&win, 100);

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
			grid.update(TPF);
		}
		win.clear(sf::Color::White);
		grid.render();
		win.display();
	}
	return 0;
}