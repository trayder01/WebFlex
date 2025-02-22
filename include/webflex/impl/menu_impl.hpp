#ifndef MENUIMPL_HPP
#define MENUIMPL_HPP

#include <QMenu>
#include <QPainterPath>
#include <QMimeData>

namespace webflex::impl
{
    class RoundedMenu : public QMenu
    {
    public:
        inline explicit RoundedMenu(QWidget *parent = nullptr);

        float radius() const {
            return m_radius;
        }

        void setRadius(float radius) {
            m_radius = radius;
        } 

    protected:
        inline void resizeEvent(QResizeEvent *event) override;

        inline void focusOutEvent(QFocusEvent *event) override;

    private:
        float m_radius = 0.0;
    };

    inline RoundedMenu::RoundedMenu(QWidget *parent) : QMenu(parent)
    {
        setWindowFlags(Qt::Popup | Qt::FramelessWindowHint); 
        setAttribute(Qt::WA_TranslucentBackground);          

        setStyleSheet(
            "QMenu {"
            "background-color:rgba(48, 48, 48, 0.93);"
            // "border: 2px solid gray;"
            "border-radius: 8px;" 
            "}"
            "QMenu::item {"
            "padding: 8px 20px;"
            "}"
            "QMenu::item:selected {"
            "background-color: white;"
            "border-radius: 8px;"
            "color: black;"
            "}"
        );

        setRadius(5);
    }

    inline void RoundedMenu::resizeEvent(QResizeEvent *event) 
    {
        QMenu::resizeEvent(event);

        QPainterPath path;
        path.addRoundedRect(rect(), radius(), radius());
        QRegion region(path.toFillPolygon().toPolygon());
        setMask(region); 
    }

    inline void RoundedMenu::focusOutEvent(QFocusEvent *event) 
    {
        this->hide();
        QMenu::focusOutEvent(event);
    }
}

#endif // MENUIMPL_HPP