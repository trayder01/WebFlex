#ifndef COLOR_HPP
#define COLOR_HPP

#include <string>
#include <memory>

namespace webflex {
    namespace impl {
        struct ColorImpl;
    }

    struct Rgba {
        int r;
        int g;
        int b;
        int a;
    };

    class Color 
    {
    public:
        Color();

        explicit Color(const char* color);

        explicit Color(const std::string& colorName);
        
        explicit Color(const Rgba& color);

        explicit Color(int r, int g, int b, int a = 255);

        Color(const Color& other);
        Color& operator=(const Color& other);

        bool operator==(const Color& other) const;
        bool operator!=(const Color& other) const;

        ~Color();

        std::string name() const;

        void setRgba(const Rgba& color);

        int red() const;

        int blue() const;
        
        int green() const;
        
        int alpha() const;

    private:
        std::unique_ptr<impl::ColorImpl> m_impl;
    };
}

#endif // COLOR_HPP