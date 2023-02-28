#include <iostream>
#include <map>

#include "Boid.hpp"
#include "Obstacle.hpp"

#include <SFML/Graphics.hpp>

class Flock
{
public:
	Flock(int size, sf::RenderWindow* win, int boid_vision)
		: m_window(win)
		, m_cell_size(1.25f * boid_vision)
		, m_sprites(sf::Triangles)
	{
		std::vector<sf::FloatRect> bounds;
		std::unique_ptr<Obstacle> ptr = std::make_unique<Circle>(100, sf::Vector2f(500, 500));
		bounds.push_back(ptr->getBounds());
		m_colliders.push_back(std::move(ptr));
		ptr = std::make_unique<Circle>(50, sf::Vector2f(300, 700));
		bounds.push_back(ptr->getBounds());
		m_colliders.push_back(std::move(ptr));
		ptr = std::make_unique<Circle>(70, sf::Vector2f(400, 200));
		bounds.push_back(ptr->getBounds());
		m_colliders.push_back(std::move(ptr));
		ptr = std::make_unique<Rectangle>(sf::FloatRect(800, 100, 30, 300));
		bounds.push_back(ptr->getBounds());
		m_colliders.push_back(std::move(ptr));
		ptr = std::make_unique<Rectangle>(sf::FloatRect(0, -50, 1000, 100));
		bounds.push_back(ptr->getBounds());
		m_colliders.push_back(std::move(ptr));
		ptr = std::make_unique<Rectangle>(sf::FloatRect(950, 0, 100, 1000));
		bounds.push_back(ptr->getBounds());
		m_colliders.push_back(std::move(ptr));
		ptr = std::make_unique<Rectangle>(sf::FloatRect(-50, 0, 100, 1000));
		bounds.push_back(ptr->getBounds());
		m_colliders.push_back(std::move(ptr));
		ptr = std::make_unique<Rectangle>(sf::FloatRect(0, 950, 1000, 100));
		bounds.push_back(ptr->getBounds());
		m_colliders.push_back(std::move(ptr));

		for (int i = 0; i < size; i++)
		{
			Boid boid(800 + rand() % 300, 100 + rand() % 200, boid_vision, 130, 2.f * boid_vision, 10.f);
			boid.setRotation(rand() % 360);
			sf::Vector2f pos = sf::Vector2f(rand() % 1000, rand() % 1000);
			bool reject = true;
			while (reject)
			{
				pos = sf::Vector2f(rand() % 1000, rand() % 1000);
				reject = false;
				for (auto& i : bounds)
					if (i.contains(pos))
						reject = true;
			}
			boid.setPosition(pos);
			m_boids.push_back(boid);
		}
	}

	void update(sf::Time dt)
	{
		m_map.clear();
		for (auto& i : m_boids)
		{
			i.turnOff();
			i.setThruster(true, 1.f);
			sf::Vector2f pos = i.getPosition();
			std::pair<int, int> coord(std::floor(pos.x / m_cell_size), std::floor(pos.y / m_cell_size));
			m_map[coord].push_back(&i);
		}

		for (auto& i : m_boids)
		{
			sf::Vector2f pos = i.getPosition();
			std::pair<int, int> coord(std::floor(pos.x / m_cell_size), std::floor(pos.y / m_cell_size));
			for (int x = coord.first - 1; x <= coord.first + 1; x++)
				for (int y = coord.second - 1; y <= coord.second + 1; y++)
				{
					std::pair<int, int> cell(x, y);
					if (m_map.find(cell) != m_map.end())
						i.updateData(m_map[cell]);
				}
		}

		for (auto& i : m_boids)
		{
			i.updateFeeler(m_colliders);
			i.update(dt);
			sf::Vector2f pos = i.getPosition();
			if (pos.x < 0)
				pos.x += 1000;
			else if (pos.x > 1000)
				pos.x -= 1000;
			if (pos.y < 0)
				pos.y += 1000;
			else if (pos.y > 1000)
				pos.y -= 1000;
			i.setPosition(pos);
		}
	}

	void render()
	{
		for (int i = 0; i < m_colliders.size(); i++)
			m_window->draw(*m_colliders[i]);
		m_sprites.clear();
		for (auto& i : m_boids)
			i.batchVertices(m_sprites);
		m_window->draw(m_sprites);
	}
private:
	sf::RenderWindow* m_window;
	std::vector<Boid> m_boids;
	int m_cell_size;
	std::map<std::pair<int, int>, std::vector<Boid*>> m_map;
	sf::VertexArray m_sprites;
	std::vector<std::unique_ptr<Obstacle>> m_colliders;
};

int main()
{
	srand(time(0));
	sf::RenderWindow win(sf::VideoMode(1000, 1000), "HI", sf::Style::None);
	sf::Clock clock;
	sf::Time elapsed = clock.restart();
	sf::Time total = elapsed;
	sf::Time TPF = sf::seconds(1.f / 60);

	Flock bao(200, &win, 40);
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
		bao.render();
		win.display();
	}
	return 0;
}