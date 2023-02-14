#include "Bersenham_line.hpp"

std::vector<sf::Vector2i> bersenham_line(sf::Vector2i start, sf::Vector2i end)
{
	sf::Vector2i delta = end - start;
	if (abs(delta.x) >= abs(delta.y))
	{
		sf::Vector2i step;
		step.x = 2 * (delta.x > 0) - 1;
		step.y = 2 * (delta.y > 0) - 1;
		delta.x = abs(delta.x);
		delta.y = abs(delta.y);
		std::vector<sf::Vector2i> ans = { start };
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