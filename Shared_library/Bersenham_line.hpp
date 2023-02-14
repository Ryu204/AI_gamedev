#ifndef AI_BERSENHAM_LINE
#define AI_BERSENHAM_LINE

#include <vector>

#include <SFML/Graphics.hpp>

std::vector<sf::Vector2i> bersenham_line(sf::Vector2i start, sf::Vector2i end);

#endif