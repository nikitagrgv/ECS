#pragma once

#include "SFML/Graphics/Rect.hpp"
#include "SFML/System/Vector2.hpp"
#include <cmath>

namespace Math
{

template<class T>
inline auto length(T vec)
{
    return std::sqrt(vec.x * vec.x + vec.y * vec.y);
}

template<class T>
inline T normalize(T vec)
{
    return vec / length(vec);
}

inline float toDegrees(float radians)
{
    return radians * 180.f / 3.1415926f;
}

template<class T>
inline sf::Vector2f getBottomRight(T shape)
{
    const sf::FloatRect bounds = shape.getLocalBounds();
    return {bounds.left + bounds.width, bounds.top + bounds.height};
}

template<class T>
inline sf::Vector2f getBoundCenter(T shape)
{
    const sf::FloatRect bounds = shape.getLocalBounds();
    return {bounds.width / 2, bounds.height / 2};
}

template<class T>
inline auto multiply(T vec0, T vec1)
{
    return T{vec0.x * vec1.x, vec0.y * vec1.y};
}

} // namespace Math
