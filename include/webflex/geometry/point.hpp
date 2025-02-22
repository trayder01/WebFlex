#ifndef WPOINT_HPP
#define WPOINT_HPP

#include <utility>

namespace webflex::geometry
{
    class Point 
    {
    public:
        constexpr Point();
        constexpr Point(int x, int y);
        constexpr Point(const Point& other);
        constexpr Point& operator=(const Point& other);
        constexpr Point(Point&& other);
        constexpr Point& operator=(Point&& other);

        constexpr bool operator==(const Point& other);
        constexpr bool operator!=(const Point& other);

        constexpr Point& operator*=(const Point& other);
        constexpr Point& operator+=(const Point& other);
        constexpr Point& operator-=(const Point& other);
        constexpr Point& operator/=(const Point& other);

        constexpr bool isNull() const;

        constexpr void setX(int x);
        constexpr void setY(int y);

        constexpr int x() const;
        constexpr int y() const;

        friend std::ostream& operator<<(std::ostream& os, const Point& other) 
        {
            os << std::string("Point(") << std::to_string(other.x_) << ", " << std::to_string(other.y_) << std::string(")");
            return os;
        }

    private:
        int x_;
        int y_;
    };

    inline constexpr Point::Point() : x_{}, y_{} {}

    inline constexpr Point::Point(int w, int h) : x_{w}, y_{h} {}

    inline constexpr Point::Point(const Point& other) : x_{other.x_}, y_{other.y_} {}

    inline constexpr Point& Point::operator=(const Point& other)
    {
        if (this != &other) 
        {
            this->x_ = other.x_;
            this->y_ = other.y_;
        }

        return *this;
    }

    inline constexpr Point::Point(Point&& other) : x_{std::move(other.x_)}, y_{std::move(other.y_)} {}

    inline constexpr Point& Point::operator=(Point&& other) 
    {
        if (this != &other) 
        {
            this->y_ = std::move(other.y_);
            this->x_ = std::move(other.x_);
        }

        return *this;
    }

    inline constexpr bool Point::operator==(const Point& other) 
    {
        return this->y_ == other.y_ && this->x_ == other.x_;
    }

    inline constexpr bool Point::operator!=(const Point& other) 
    {
        return !(*this == other);
    }

    inline constexpr Point& Point::operator*=(const Point& other)
    {
        this->x_ *= other.x_;
        this->y_ *= other.y_;
        return *this;
    }

    inline constexpr Point& Point::operator+=(const Point& other) 
    {
        this->x_ += other.x_;
        this->y_ += other.y_;
        return *this;
    }
    
    inline constexpr Point& Point::operator-=(const Point& other) 
    {
        this->x_ -= other.x_;
        this->y_ -= other.y_;
        return *this;
    }
    
    inline constexpr Point& Point::operator/=(const Point& other) 
    {
        this->x_ /= other.x_;
        this->y_ /= other.y_;
        return *this;
    }

    inline constexpr bool Point::isNull() const 
    {
        return x() == 0 && y() == 0;
    }

    inline constexpr void Point::setX(int x) 
    {
        x_ = x;
    }

    inline constexpr void Point::setY(int y) 
    {
        y_ = y;
    }

    inline constexpr int Point::x() const 
    {
        return x_;
    }

    inline constexpr int Point::y() const 
    {
        return y_;
    }
}

#endif // WPOINT_HPP