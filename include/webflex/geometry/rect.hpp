#ifndef WRECT_HPP
#define WRECT_HPP

#include "point.hpp"
#include "size.hpp"

namespace webflex::geometry
{
    class Rect
    {
    public:
        constexpr Rect() = default;
        constexpr Rect(double x, double y, double width, double height)
            : m_top_left(x, y), m_size(width, height) {}

        constexpr Rect(const Point &top_left, const Size &size)
            : m_top_left(top_left), m_size(size) {}

        constexpr double x() const noexcept { return m_top_left.x(); }
        constexpr double y() const noexcept { return m_top_left.y(); }
        constexpr double width() const noexcept { return m_size.width(); }
        constexpr double height() const noexcept { return m_size.height(); }

        constexpr Point topLeft() const noexcept { return m_top_left; }
        constexpr Size size() const noexcept { return m_size; }

        constexpr void setX(double x) noexcept { m_top_left.setX(x); }
        constexpr void setY(double y) noexcept { m_top_left.setY(y); }
        constexpr void setWidth(double width) noexcept { m_size.setWidth(width); }
        constexpr void setHeight(double height) noexcept { m_size.setHeight(height); }

        constexpr void setTopLeft(const Point &point) noexcept { m_top_left = point; }
        constexpr void setSize(const Size &size) noexcept { m_size = size; }

        constexpr double area() const noexcept { return m_size.width() * m_size.height(); }
        constexpr bool contains(const Point &point) const noexcept
        {
            return point.x() >= x() && point.x() <= x() + width() &&
                   point.y() >= y() && point.y() <= y() + height();
        }

        constexpr bool intersects(const Rect &other) const noexcept
        {
            return x() < other.x() + other.width() && x() + width() > other.x() &&
                   y() < other.y() + other.height() && y() + height() > other.y();
        }

        constexpr Rect &translate(double dx, double dy) noexcept
        {
            m_top_left.setX(m_top_left.x() + dx);
            m_top_left.setY(m_top_left.y() + dy);
            return *this;
        }

    private:
        Point m_top_left = {0.0, 0.0};
        Size m_size = {0.0, 0.0};
    };

} // namespace webflex

#endif // WRECT_HPP
