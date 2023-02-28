#ifndef AI_SHARED_UTILISE
#define AI_SHARED_UTILISE

#include <cmath>
#include <cassert>
#include <SFML/Graphics.hpp>

namespace Utilise
{
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
	T product(sf::Vector2<T> a, sf::Vector2<T> b)
	{
		return a.x * b.x + a.y * b.y;
	}

	template<typename T>
	T lerp(T start, T end, float t)
	{
		return start + (end - start) * t;
	}

	const float PI = 22.f / 7;

	inline float toDegree(float radian)
	{
		return radian * 180 / PI;
	}

	inline float toRadian(float degree)
	{
		return degree * PI / 180;
	}
}
#endif