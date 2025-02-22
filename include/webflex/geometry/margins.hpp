#ifndef WMARGINS_HPP
#define WMARGINS_HPP

namespace webflex::geometry {

    class margins {
    public:
        constexpr margins() = default;
        constexpr margins(double left, double top, double right, double bottom)
            : m_left(left), m_top(top), m_right(right), m_bottom(bottom) {}

        constexpr double left() const noexcept { return m_left; }
        constexpr double top() const noexcept { return m_top; }
        constexpr double right() const noexcept { return m_right; }
        constexpr double bottom() const noexcept { return m_bottom; }

        constexpr void setLeft(double left) noexcept { m_left = left; }
        constexpr void setTop(double top) noexcept { m_top = top; }
        constexpr void setRight(double right) noexcept { m_right = right; }
        constexpr void setBottom(double bottom) noexcept { m_bottom = bottom; }

        constexpr margins &operator+=(const margins &margins) noexcept {
            m_left += margins.m_left;
            m_top += margins.m_top;
            m_right += margins.m_right;
            m_bottom += margins.m_bottom;
            return *this;
        }

        constexpr margins &operator-=(const margins &margins) noexcept {
            m_left -= margins.m_left;
            m_top -= margins.m_top;
            m_right -= margins.m_right;
            m_bottom -= margins.m_bottom;
            return *this;
        }

        constexpr margins &operator+=(int value) noexcept {
            m_left += value;
            m_top += value;
            m_right += value;
            m_bottom += value;
            return *this;
        }

        constexpr margins &operator-=(int value) noexcept {
            m_left -= value;
            m_top -= value;
            m_right -= value;
            m_bottom -= value;
            return *this;
        }

        constexpr margins &operator*=(int value) noexcept {
            m_left *= value;
            m_top *= value;
            m_right *= value;
            m_bottom *= value;
            return *this;
        }

        constexpr margins &operator/=(int value) {
            m_left /= value;
            m_top /= value;
            m_right /= value;
            m_bottom /= value;
            return *this;
        }

        constexpr margins &operator*=(double value) noexcept {
            m_left *= value;
            m_top *= value;
            m_right *= value;
            m_bottom *= value;
            return *this;
        }

        constexpr margins &operator/=(double value) {
            m_left /= value;
            m_top /= value;
            m_right /= value;
            m_bottom /= value;
            return *this;
        }

    private:
        double m_left = 0.0;
        double m_top = 0.0;
        double m_right = 0.0;
        double m_bottom = 0.0;
    };

} // namespace webflex

#endif // WMARGINS_HPP
