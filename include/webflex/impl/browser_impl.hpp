#ifndef WBROWSERIMPL_HPP
#define WBROWSERIMPL_HPP

#include <QWebEngineProfile>
#include <QWebEngineView>
#include <QMainWindow>
#include <QWebChannel>
#include <QVBoxLayout>
#include <QSplitter>
#include <QMenu>

//
#include "menu_impl.hpp"
#include "devtools_view.hpp"
#include "../core/invoker.hpp"
#include "../core/wobjectcpp.h"
#include "../core/wobjectimpl.h"
//

namespace webflex::impl
{
    class PageImpl;
    class BrowserImpl : public QMainWindow
    {
        W_OBJECT(BrowserImpl)
    public:
        explicit BrowserImpl(QWidget *parent = nullptr);
        ~BrowserImpl();

        void resized(int w, int h) W_SIGNAL(resized, w, h) 
        void maximize(bool state) W_SIGNAL(maximize, state) 
        void minimize(bool state) W_SIGNAL(minimize, state) 
        void close_() W_SIGNAL(close_) 
        void closed() W_SIGNAL(closed) 
        void zoomChanged(double factor) W_SIGNAL(zoomChanged, factor)

        core::Invoker *invoker() const { return m_invoker.get(); }
        QWebEngineView *view() const { return m_view.get(); }
        QWebChannel *channel() const { return m_channel.get(); }

    private:
        void initMenu();

        void createFileMenu(QMenuBar *menuBar);

        void createEditMenu(QMenuBar *menuBar);

        void createViewMenu(QMenuBar *menuBar);

        void createWindowMenu(QMenuBar *menuBar);

        void createHelpMenu(QMenuBar *menuBar);

        void createDevToolsMenu(QMenu *parentMenu);

        void addActionWithShortcut(QMenu *menu, const QString &actionName, const QKeySequence &shortcut, const std::function<void()> &slot);

        void adjustZoom(float delta);

        void injectCustomScripts();

        void moveDeveloperToolsLeft();
        W_SLOT(moveDeveloperToolsLeft)

        void moveDeveloperToolsRight();
        W_SLOT(moveDeveloperToolsRight)

        void moveDeveloperToolsTop();
        W_SLOT(moveDeveloperToolsTop)

        void moveDeveloperToolsBottom();
        W_SLOT(moveDeveloperToolsBottom)

        void openDeveloperToolsWindow();
        W_SLOT(openDeveloperToolsWindow)

    protected:
        void resizeEvent(QResizeEvent *event) override;
        void changeEvent(QEvent *event) override;
        void closeEvent(QCloseEvent *event) override;
        void keyPressEvent(QKeyEvent *event) override;

    private:
        QString scripts() const;

    private:
        std::unique_ptr<QWebEngineView> m_view = nullptr;
        std::unique_ptr<QWebChannel> m_channel = nullptr;
        std::unique_ptr<QWebEngineView> m_developer_tools_view = nullptr;
        std::unique_ptr<core::Invoker> m_invoker = nullptr;
        std::unique_ptr<QWebEnginePage> m_developer_tools_page = nullptr;
        std::unique_ptr<QVBoxLayout> m_central_layout = nullptr;
        std::unique_ptr<DevToolsView> m_developer_tools_window = nullptr;
        std::unique_ptr<QWebEngineProfile> m_profile = nullptr;

        std::unique_ptr<PageImpl> m_page;
        QSplitter *m_main_splitter = nullptr;
        QSplitter *m_side_splitter = nullptr;
        QWidget *m_central_widget = nullptr;

        bool is_open_developer_tools_window = false;
        bool is_open_developer_tools_view = false;

        struct MenuList
        {
            std::unique_ptr<RoundedMenu> m_edit_menu = nullptr;
            std::unique_ptr<RoundedMenu> m_file_menu = nullptr;
            std::unique_ptr<RoundedMenu> m_view_menu = nullptr;
            std::unique_ptr<RoundedMenu> m_window_menu = nullptr;
            std::unique_ptr<RoundedMenu> m_dev_tools_menu = nullptr;
            std::unique_ptr<RoundedMenu> m_help_menu = nullptr;
        } m_menu_list;
    };
}

#endif // WBROWSERIMPL_HPP