#include "webflex/color.hpp"
#include <QColor>

#include <iostream>

namespace webflex::impl {
    struct ColorImpl 
    {
        QColor m_color;
    };
}

namespace webflex 
{
    Color::Color() : m_impl(new impl::ColorImpl) {}

    Color::Color(const char* color) : Color(std::string(color)) 
    {

    }

    Color::Color(const std::string& colorName) : m_impl(new impl::ColorImpl)
    {
        m_impl->m_color = QColor(colorName.c_str());
    }

    Color::Color(const Rgba& color) : Color(color.r, color.g, color.b, color.a)
    {
    }

    Color::Color(int r, int g, int b, int a) : m_impl(new impl::ColorImpl)
    {
        m_impl->m_color = QColor(r, g, b, a);
    }

    Color::Color(const Color& other) 
        : m_impl(new impl::ColorImpl(*other.m_impl)) {}

    Color& Color::operator=(const Color& other) {
        if (this != &other) {
            m_impl = std::make_unique<impl::ColorImpl>(*other.m_impl);
        }
        return *this;
    }

    bool Color::operator==(const Color& other) const {
        return m_impl->m_color == other.m_impl->m_color;
    }
    
    bool Color::operator!=(const Color& other) const {
        return !(*this == other);
    }

    Color::~Color() 
    {
        if (m_impl) {
            m_impl.reset();
        }
    }

    void Color::setRgba(const Rgba& color) 
    {
        m_impl->m_color = QColor(color.r, color.g, color.b, color.a);
    }

    std::string Color::name() const 
    {
        return m_impl->m_color.name().toStdString();
    }

    int Color::red() const {
        return m_impl->m_color.red();
    }

    int Color::blue() const {
        return m_impl->m_color.blue();
    }
    
    int Color::green() const {
        return m_impl->m_color.green();
    }
    
    int Color::alpha() const {
        return m_impl->m_color.alpha();
    }
}