#ifndef WSIZE_HPP
#define WSIZE_HPP

#include <utility>

namespace webflex::geometry {
    class Size 
    {
    public:
        inline constexpr Size();
        inline constexpr Size(int w, int h);
        inline constexpr Size(const Size& other);
        inline constexpr Size& operator=(const Size& other);
        inline constexpr Size(Size&& other);
        inline constexpr Size& operator=(Size&& other);

        inline constexpr Size& operator+=(const Size& size);
        inline constexpr Size& operator-=(const Size& size);        

        inline constexpr bool operator==(const Size& other);
        inline constexpr bool operator!=(const Size& other);

        inline constexpr void setWidth(int w);
        inline constexpr void setHeight(int h);

        inline constexpr int width() const;
        inline constexpr int height() const;

        inline constexpr bool isEmpty() const;

        inline constexpr bool isValid() const;

        friend std::ostream& operator<<(std::ostream& os, const Size& other) 
        {
            os << std::string("Size(") << std::to_string(other.m_w) << ", " << std::to_string(other.m_h) << std::string(")");
            return os;
        }

    private:
        int m_w;
        int m_h;
    };

    inline constexpr Size::Size() : m_w{}, m_h{} {}

    inline constexpr Size::Size(int w, int h) : m_w{w}, m_h{h} {}

    inline constexpr Size::Size(const Size& other) : m_w{other.m_w}, m_h{other.m_h} {}

    inline constexpr Size& Size::operator=(const Size& other)
    {
        if (this != &other) 
        {
            this->m_w = other.m_w;
            this->m_h = other.m_h;
        }

        return *this;
    }

    inline constexpr Size::Size(Size&& other) : m_w{std::move(other.m_w)}, m_h{std::move(other.m_h)} {}

    inline constexpr Size& Size::operator=(Size&& other) 
    {
        if (this != &other) 
        {
            this->m_h = std::move(other.m_h);
            this->m_w = std::move(other.m_w);
        }

        return *this;
    }

    inline constexpr Size& Size::operator+=(const Size& size) 
    {
        this->m_w += size.m_w;
        this->m_h += size.m_h;

        return *this;
    }

    inline constexpr Size& Size::operator-=(const Size& size) 
    {
        this->m_w += size.m_w;
        this->m_h += size.m_h;
        return *this;
    }

    inline constexpr bool Size::operator==(const Size& other) 
    {
        return this->m_h == other.m_h && this->m_w == other.m_w;
    }

    inline constexpr bool Size::operator!=(const Size& other) 
    {
        return !(*this == other);
    }

    inline constexpr void Size::setWidth(int w) 
    {
        m_w = w;
    }

    inline constexpr void Size::setHeight(int h) 
    {
        m_h = h;
    }

    inline constexpr int Size::width() const 
    {
        return m_w;
    }

    inline constexpr int Size::height() const 
    {
        return m_h;
    }

    inline constexpr bool Size::isEmpty() const
    {
        return m_w == 0 && m_h == 0;
    }

    inline constexpr bool Size::isValid() const 
    {
        return m_w >= 0 && m_h >= 0;
    }
}

#endif // WSIZE_HPP