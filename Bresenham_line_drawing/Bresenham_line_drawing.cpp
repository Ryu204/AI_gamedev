#include <iostream>
#include <vector>
#include <cassert>
#include <cmath>

#include <SFML/Graphics.hpp>

#include "Utilise.hpp"

const int SCREEN_SIZE = 1000;

std::vector<sf::Vector2u> bersenham_line(sf::Vector2u start, sf::Vector2u end)
{
	sf::Vector2i delta(end.x - start.x, end.y - start.y);
	if (abs(delta.x) >= abs(delta.y))
	{
		sf::Vector2i step;
		step.x = 2 * (delta.x > 0) - 1;
		step.y = 2 * (delta.y > 0) - 1;
		delta.x = abs(delta.x); 
		delta.y = abs(delta.y);
		std::vector<sf::Vector2u> ans = { start };
		int p = delta.x - 2 * delta.y;
		while (start != end)
		{
			start.x += step.x;
			if (p <= 0)
			{
				start.y += step.y;
				p += 2 * delta.x - 2 * delta.y;
			}
			else
				p += -2 * delta.y;
			ans.push_back(start);
		}
		return ans;
	}
	else
	{
		std::swap(start.x, start.y);
		std::swap(end.x, end.y);
		auto ans = bersenham_line(start, end);
		for (auto& i : ans)
			std::swap(i.x, i.y);
		return ans;
	}
}

class Chaser
{
public:
	Chaser(sf::Vector2u pos, float speed)
		: m_move_interval(sf::seconds(1.f / (abs(speed) + 1)))
		, m_path(1, pos)
		, m_current_index(0)
	{ }

	void update(sf::Time dt, sf::Vector2u mouse)
	{
		if (mouse != *m_path.rbegin())
			setGoal(mouse);
		m_elapsed_time += dt;
		while (m_elapsed_time >= m_move_interval)
		{
			m_current_index = std::min<int>(m_current_index + 1, m_path.size() - 1);
			m_elapsed_time -= m_move_interval;
		}
	}

	sf::Vector2u getCoords() const
	{
		return m_path[m_current_index];
	}
private:
	void setGoal(sf::Vector2u mouse)
	{
		sf::Vector2u start = m_path[m_current_index];
		m_path = bersenham_line(start, mouse);
		m_current_index = 0;
	}
private:
	sf::Time m_move_interval;
	sf::Time m_elapsed_time;
	std::vector<sf::Vector2u> m_path;
	unsigned int m_current_index;
};

class Grid
{
public:
	Grid(sf::RenderWindow* win, unsigned int size, float chaser_speed, int chaser_number)
		: m_window(win)
		, m_size(size)
		, m_cell_size(SCREEN_SIZE * 1.f / size)
		, m_lines(sf::Lines)
		, m_cursor(m_cell_size * 0.75f * 0.5f, 8)
		, m_chaser_body(chaser_number, sf::CircleShape(m_cell_size / 2.f, 3))
	{ 
		assert(size > 1 && "Must have at least 4 cells");
		Utilise::center(m_cursor);
		m_cursor.setFillColor(sf::Color::Red);

		for (int i = 1; i <= chaser_number; i++)
			m_chasers.push_back(Chaser(sf::Vector2u(rand() % m_size, rand() % m_size), chaser_speed));

		for (int i = 0; i <= size; i++)
		{
			m_lines.append(sf::Vertex{ sf::Vector2f(i * m_cell_size, 0) });
			m_lines.append(sf::Vertex{ sf::Vector2f(i * m_cell_size, SCREEN_SIZE) });
			m_lines.append(sf::Vertex{ sf::Vector2f(0, i * m_cell_size) });
			m_lines.append(sf::Vertex{ sf::Vector2f(SCREEN_SIZE, i * m_cell_size) });
		}

		for (int i = 0; i < m_lines.getVertexCount(); i++)
			m_lines[i].color = sf::Color::Black;

		for (auto& i : m_chaser_body)
		{
			i.setFillColor(sf::Color::Black);
			Utilise::center(i);
		}
	};

	void update(sf::Time dt)
	{
		sf::Vector2i mouse = sf::Mouse::getPosition(*m_window);
		mouse = sf::Vector2i(sf::Vector2u(mouse.x * 1.f / m_cell_size, mouse.y * 1.f / m_cell_size));
		mouse.x = std::min<int>(std::max<int>(mouse.x, 0), m_size - 1);
		mouse.y = std::min<int>(std::max<int>(mouse.y, 0), m_size - 1);
		if (sf::Vector2u(mouse) != m_mouse_coords)
		{
			m_mouse_coords = sf::Vector2u(mouse);
			m_trace.clear();
			sf::RectangleShape prefab(sf::Vector2f(m_cell_size, m_cell_size));
			prefab.setFillColor(sf::Color::Green);
			for (auto& i : m_chasers)
				for (auto& p : bersenham_line(i.getCoords(), m_mouse_coords))
				{
					prefab.setPosition(p.x * m_cell_size, p.y * m_cell_size);
					m_trace.push_back(prefab);
				}
		}
		m_cursor.setPosition(m_cell_size * (m_mouse_coords.x + 0.5f), m_cell_size * (m_mouse_coords.y + 0.5f));

		for (int i = 0; i < m_chasers.size(); i++)
		{
			m_chasers[i].update(dt, m_mouse_coords);
			auto chaser_coords = m_chasers[i].getCoords();
			m_chaser_body[i].setPosition(m_cell_size * (chaser_coords.x + 0.5f), m_cell_size * (chaser_coords.y + 0.5f));
		}
	}

	void render()
	{
		for (auto& i : m_trace)
			m_window->draw(i);
		for (auto& i : m_chaser_body)
			m_window->draw(i);
		m_window->draw(m_lines);
		m_window->draw(m_cursor);
	}

	sf::Vector2u getMouseCoords() const
	{
		return m_mouse_coords;
	}
private:
	sf::RenderWindow* m_window;
	unsigned int m_size;
	float m_cell_size;
	sf::Vector2u m_mouse_coords;
	sf::VertexArray m_lines;
	sf::CircleShape m_cursor;

	std::vector<Chaser> m_chasers;
	std::vector<sf::CircleShape> m_chaser_body;

	std::vector<sf::RectangleShape> m_trace;
};

int main()
{
	srand(time(0));
	std::cout << "Enter number of rows and columns.";
	int row = 0;
	std::cin >> row;
	std::cout << "Enter number of chasers.";
	int chaser = 0;
	std::cin >> chaser;

	sf::RenderWindow win(sf::VideoMode(SCREEN_SIZE, SCREEN_SIZE), "HI", sf::Style::None);
	win.setFramerateLimit(100);
	sf::Clock clock;
	sf::Time elapsed = clock.restart();
	sf::Time TPF = sf::seconds(1.f / 60);

	Grid grid(&win, row, row / 10.f, chaser);

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