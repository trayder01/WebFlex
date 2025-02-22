#include "webflex/impl/browser_impl.hpp"
#include "webflex/impl/page_impl.hpp"
#include "webflex/core/wobjectimpl.h"
#include "webflex/application.hpp"

#include <QWebEngineScriptCollection>
#include <QWebEngineCookieStore>
#include <QWebEngineSettings>
#include <QWebEngineScript>
#include <QResizeEvent>
#include <QMenuBar>
#include <QEvent>
#include <QFile>
#include <QDir>

namespace webflex
{
    namespace impl
    {
        DevToolsView::DevToolsView(QWidget *parent)
        : QWebEngineView(parent)
        , zoomInShortcutPlus(new QShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Plus), this))
        , zoomInShortcutEqual(new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Equal), this))
        , zoomOutShortcut(new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Minus), this))
        , resetZoomShortcut(new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_0), this))
        {
            setupShortcuts();
        }

        DevToolsView::~DevToolsView() 
        {
        }

        void DevToolsView::setupShortcuts()
        {
            QObject::connect(zoomInShortcutPlus.data(), &QShortcut::activated, this, &DevToolsView::zoomIn);
            QObject::connect(zoomInShortcutEqual.data(), &QShortcut::activated, this, &DevToolsView::zoomIn);
            QObject::connect(zoomOutShortcut.data(), &QShortcut::activated, this, &DevToolsView::zoomOut);
            QObject::connect(resetZoomShortcut.data(), &QShortcut::activated, this, &DevToolsView::resetZoom);
        }

        void DevToolsView::zoomIn()
        {
            auto currentZoom = page()->zoomFactor();
            currentZoom = qMin(currentZoom + 0.1, 3.0);
            page()->setZoomFactor(currentZoom);
        }

        void DevToolsView::zoomOut()
        {
            auto currentZoom = page()->zoomFactor();
            currentZoom = qMax(currentZoom - 0.1, 0.5);
            page()->setZoomFactor(currentZoom);
        }

        void DevToolsView::resetZoom()
        {
            page()->setZoomFactor(1.0);
        }

        void DevToolsView::closeEvent(QCloseEvent* event) 
        {
            QWebEngineView::closeEvent(event);
            closed();
        }

        W_OBJECT_IMPL(DevToolsView)
    } // namespace impl
} // namespace webflex

namespace webflex::impl {
    BrowserImpl::BrowserImpl(QWidget *parent)
    : QMainWindow(parent)
    , m_view(std::make_unique<QWebEngineView>())  
    , m_channel(std::make_unique<QWebChannel>())
    , m_invoker(std::make_unique<core::Invoker>())
    , m_central_widget(new QWidget(this))  
    , m_main_splitter(new QSplitter(Qt::Vertical, this))  
    , m_side_splitter(new QSplitter(Qt::Horizontal, this))
    , m_central_layout(new QVBoxLayout())
    , m_developer_tools_view(std::make_unique<QWebEngineView>())
    , m_developer_tools_page(std::make_unique<QWebEnginePage>())
    , m_developer_tools_window(std::make_unique<DevToolsView>())
    {
        setCentralWidget(m_central_widget);
        m_profile = std::make_unique<QWebEngineProfile>("WebFlexProfile", m_view.get());
        m_page = std::make_unique<PageImpl>(m_profile.get());

        m_profile->setPersistentCookiesPolicy(QWebEngineProfile::AllowPersistentCookies);

        auto cookiePath = webflex::Application::getPath(webflex::PathKey::CookiePath);
        auto cachePath = webflex::Application::getPath(webflex::PathKey::CachePath);
        auto applicationDir = webflex::Application::getPath(webflex::PathKey::ApplicationDir);

        auto cookieDir = QString::fromStdString(
            !cookiePath.empty() ? cookiePath :
            !applicationDir.empty() ? applicationDir :
            QCoreApplication::applicationDirPath().toStdString()) + "/cookie";

        auto cacheDir = QString::fromStdString(
            !cachePath.empty() ? cachePath :
            !applicationDir.empty() ? applicationDir :
            QCoreApplication::applicationDirPath().toStdString()) + "/cache";

        auto downloadsPath = webflex::Application::getPath(webflex::PathKey::DownloadsPath);

        m_profile->setDownloadPath(!downloadsPath.empty() ? QString::fromStdString(downloadsPath) + "/" : QCoreApplication::applicationDirPath() + "/");
        
        QDir dir(cookieDir);
        if (!dir.exists()) {
            dir.mkpath(".");
        }

        dir.setPath(cacheDir);
        if (!dir.exists()) {
            dir.mkpath(".");
        }

        m_profile->setPersistentStoragePath(cookieDir);
        m_profile->setCachePath(cacheDir);

        m_view->settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);

        m_developer_tools_window->setWindowTitle("DevTools");

        m_central_widget->setLayout(m_central_layout.get());

        m_central_layout->setContentsMargins(0, 0, 0, 0);
        m_central_layout->setSpacing(0);

        m_view->setPage(m_page.get());
        m_view->setHtml(R"html(<html></html>)html");
        m_view->page()->setWebChannel(m_channel.get());

        m_developer_tools_view->setPage(m_developer_tools_page.get());

        m_side_splitter->addWidget(m_view.get());  
        m_side_splitter->addWidget(m_developer_tools_view.get());

        m_side_splitter->setStretchFactor(0, 3);
        m_side_splitter->setStretchFactor(1, 1);

        m_main_splitter->addWidget(m_side_splitter);
        m_central_layout->addWidget(m_main_splitter);

        m_channel->registerObject(m_invoker->objectName(), m_invoker.get());

        QObject::connect(m_developer_tools_window.get(), &DevToolsView::closed, [this]()
        {
            m_developer_tools_window->page()->setDevToolsPage(nullptr);
            is_open_developer_tools_window = false;
        });

        QObject::connect(this, &BrowserImpl::close_, [this]()
        {
            if (m_developer_tools_window) 
            {
                if (m_developer_tools_window->isEnabled()) 
                {
                    m_developer_tools_window->close();
                }
            }
        });

        injectCustomScripts();
        initMenu();
    }

    BrowserImpl::~BrowserImpl() 
    {
    }

    void BrowserImpl::resizeEvent(QResizeEvent *event)
    {
        auto size = event->size();
        resized(size.width(), size.height());
        QMainWindow::resizeEvent(event);
    }

    void BrowserImpl::changeEvent(QEvent *event)
    {
        if (event->type() == QEvent::WindowStateChange)
        {
            auto state = this->windowState();

            if (state & Qt::WindowMinimized)
            {
                minimize(true);
            }
            else if (state & Qt::WindowMaximized)
            {
                maximize(true);
            }
            else if (state == Qt::WindowNoState)
            {
                maximize(false);
                minimize(false);
            }
        }

        QMainWindow::changeEvent(event);
    }

    void BrowserImpl::closeEvent(QCloseEvent *event)
    {
        closed();
        QMainWindow::closeEvent(event);
        close_();
    }

    void BrowserImpl::keyPressEvent(QKeyEvent *event)
    {
        if (event->modifiers() & Qt::ControlModifier)
        {
            auto currentZoom = m_view->page()->zoomFactor();

            if (event->key() == Qt::Key_Plus || event->key() == Qt::Key_Equal)
            {
                m_view->page()->setZoomFactor(currentZoom + 0.1);
                zoomChanged(currentZoom);
            }
        }
        else
        {
            QMainWindow::keyPressEvent(event);
        }
    }

    QString BrowserImpl::scripts() const
    {
        QFile qwebchannel(":/qtwebchannel/qwebchannel.js");

        if (!qwebchannel.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            qFatal() << qwebchannel.errorString();
        }

        auto data = qwebchannel.readAll();

        if (data.isEmpty())
        {
            qFatal() << "data is empty";
        }

        qwebchannel.close();

        return data;
    }

    void BrowserImpl::injectCustomScripts()
    {
        QWebEngineScript script;
        script.setSourceCode(scripts());
        script.setInjectionPoint(QWebEngineScript::DocumentReady);
        script.setWorldId(QWebEngineScript::MainWorld);
        m_view->page()->scripts().insert(script);

        QWebEngineScript scriptChannel;
        scriptChannel.setSourceCode(QString(R"js(
            (() => {
                new QWebChannel(qt.webChannelTransport, (channel) => {
                    window.qchannel = channel;
                    window.invoker = channel.objects.invoker;
                });
            })();
        )js"));
        scriptChannel.setInjectionPoint(QWebEngineScript::DocumentReady);
        scriptChannel.setWorldId(QWebEngineScript::MainWorld);
        m_view->page()->scripts().insert(scriptChannel);
    }

    void BrowserImpl::initMenu()
    {
        QMenuBar *menuBar = this->menuBar();

        createFileMenu(menuBar);
        createEditMenu(menuBar);
        createViewMenu(menuBar);
        createWindowMenu(menuBar);
        createHelpMenu(menuBar);
    }

    void BrowserImpl::createFileMenu(QMenuBar *menuBar)
    {
        if (menuBar) {
            m_menu_list.m_file_menu = std::make_unique<RoundedMenu>();
            m_menu_list.m_file_menu->setTitle("File");

            QAction *quitAction = m_menu_list.m_file_menu->addAction("Quit", this, &BrowserImpl::close);
            quitAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q));

            menuBar->addMenu(m_menu_list.m_file_menu.get());
        }
    }

    void BrowserImpl::createEditMenu(QMenuBar *menuBar)
    {
        if (menuBar) {
            m_menu_list.m_edit_menu = std::make_unique<RoundedMenu>();
            m_menu_list.m_edit_menu->setTitle("Edit");

            addActionWithShortcut(m_menu_list.m_edit_menu.get(), "Undo", QKeySequence(Qt::CTRL | Qt::Key_Z), [this]() {
                m_view->page()->triggerAction(QWebEnginePage::Undo);
            });

            addActionWithShortcut(m_menu_list.m_edit_menu.get(), "Redo", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Z), [this]() {
                m_view->page()->triggerAction(QWebEnginePage::Redo);
            });

            m_menu_list.m_edit_menu->addSeparator();

            addActionWithShortcut(m_menu_list.m_edit_menu.get(), "Cut", QKeySequence(Qt::CTRL | Qt::Key_X), [this]() {
                m_view->page()->triggerAction(QWebEnginePage::Cut);
            });

            addActionWithShortcut(m_menu_list.m_edit_menu.get(), "Copy", QKeySequence(Qt::CTRL | Qt::Key_C), [this]() {
                m_view->page()->triggerAction(QWebEnginePage::Copy);
            });

            addActionWithShortcut(m_menu_list.m_edit_menu.get(), "Paste", QKeySequence(Qt::CTRL | Qt::Key_V), [this]() {
                m_view->page()->triggerAction(QWebEnginePage::Paste);
            });

            m_menu_list.m_edit_menu->addAction("Delete", this, [this]() {
                m_view->page()->runJavaScript("document.execCommand('delete');");
            });

            m_menu_list.m_edit_menu->addSeparator();

            addActionWithShortcut(m_menu_list.m_edit_menu.get(), "Select All", QKeySequence(Qt::CTRL | Qt::Key_A), [this]() {
                m_view->page()->triggerAction(QWebEnginePage::SelectAll);
            });

            menuBar->addMenu(m_menu_list.m_edit_menu.get());
        }
    }

    void BrowserImpl::createViewMenu(QMenuBar *menuBar)
    {
        if (menuBar) {
            m_menu_list.m_view_menu = std::make_unique<RoundedMenu>();
            m_menu_list.m_view_menu->setTitle("View");

            addActionWithShortcut(m_menu_list.m_view_menu.get(), "Reload", QKeySequence(Qt::CTRL | Qt::Key_R), [this](){ 
                this->m_view->reload(); 
            });

            addActionWithShortcut(m_menu_list.m_view_menu.get(), "Force Reload", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_R), [this](){ 
                this->m_page->triggerAction(QWebEnginePage::WebAction::ReloadAndBypassCache); 
            });

            m_menu_list.m_view_menu->addSeparator();
            addActionWithShortcut(m_menu_list.m_view_menu.get(), "Actual Size", QKeySequence(Qt::CTRL | Qt::Key_0), [this](){
                m_view->page()->setZoomFactor(1);
                zoomChanged(m_view->page()->zoomFactor()); 
            });

            addActionWithShortcut(m_menu_list.m_view_menu.get(), "Zoom In", QKeySequence(Qt::CTRL | Qt::Key_Plus), [this](){ 
                adjustZoom(0.1); 
            });

            addActionWithShortcut(m_menu_list.m_view_menu.get(), "Zoom Out", QKeySequence(Qt::CTRL | Qt::Key_Minus), [this](){ 
                auto currentZoom = m_view->page()->zoomFactor();

                if (currentZoom <= 0.5) {
                    currentZoom = 0.5;
                }

                m_view->page()->setZoomFactor(currentZoom - 0.1);
                zoomChanged(currentZoom); 
            });

            m_menu_list.m_view_menu->addSeparator();
            createDevToolsMenu(m_menu_list.m_view_menu.get());
            addActionWithShortcut(m_menu_list.m_view_menu.get(), "Toggle Developer Tools", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_I), [this](){ 
                moveDeveloperToolsBottom();
            });
            addActionWithShortcut(m_menu_list.m_view_menu.get(), "Close Developer Tools", QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_C), [this](){ 
                if (m_developer_tools_view->isVisible()) {
                    m_developer_tools_view->close();
                    m_view->page()->setDevToolsPage(nullptr);
                    is_open_developer_tools_view = false;
                }
                
                if (m_developer_tools_window->isVisible()) 
                {
                    m_developer_tools_window->close();
                    m_developer_tools_window->page()->setDevToolsPage(nullptr);
                    is_open_developer_tools_window = false;
                }
            });

            m_menu_list.m_view_menu->addSeparator();

            addActionWithShortcut(m_menu_list.m_view_menu.get(), "Toggle Full Screen", QKeySequence(Qt::Key_F11), [this](){ 
                setWindowState(Qt::WindowState::WindowFullScreen); 
            });

            menuBar->addMenu(m_menu_list.m_view_menu.get());
        }
    }

    void BrowserImpl::createWindowMenu(QMenuBar *menuBar)
    {
        if (menuBar) {
            m_menu_list.m_window_menu = std::make_unique<RoundedMenu>();
            m_menu_list.m_window_menu->setTitle("Window");

            addActionWithShortcut(m_menu_list.m_window_menu.get(), "Minimize", QKeySequence(Qt::CTRL | Qt::Key_M), [this](){ 
                this->setWindowState(Qt::WindowState::WindowMinimized); 
            });

            addActionWithShortcut(m_menu_list.m_window_menu.get(), "Maximize", QKeySequence(Qt::CTRL | Qt::Key_D), [this](){ 
                this->setWindowState(Qt::WindowState::WindowMaximized); 
            });

            m_menu_list.m_window_menu->addAction("Close\tCtrl+Q", this, &BrowserImpl::close);
            menuBar->addMenu(m_menu_list.m_window_menu.get());
        }
    }

    void BrowserImpl::createHelpMenu(QMenuBar *menuBar)
    {
        if (menuBar) {
            m_menu_list.m_help_menu = std::make_unique<RoundedMenu>();
            m_menu_list.m_help_menu->setTitle("Help");
            menuBar->addMenu(m_menu_list.m_help_menu.get());
        }
    }

    void BrowserImpl::createDevToolsMenu(QMenu *parentMenu)
    {
        if (parentMenu) {
            m_menu_list.m_dev_tools_menu = std::make_unique<RoundedMenu>();
            m_menu_list.m_dev_tools_menu->setTitle("Developer Tools Settings");

            auto move_to_bottom_action = m_menu_list.m_dev_tools_menu->addAction("Move To Bottom", this, &BrowserImpl::moveDeveloperToolsBottom);

            move_to_bottom_action->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S));

            auto move_to_left_action = m_menu_list.m_dev_tools_menu->addAction("Move To Left", this, &BrowserImpl::moveDeveloperToolsLeft);

            move_to_left_action->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_A));

            auto move_to_top_action = m_menu_list.m_dev_tools_menu->addAction("Move To Top", this, &BrowserImpl::moveDeveloperToolsTop);

            move_to_top_action->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_W));

            auto move_to_right_action = m_menu_list.m_dev_tools_menu->addAction("Move To Right", this, &BrowserImpl::moveDeveloperToolsRight);

            move_to_right_action->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_D));
           
            auto open_in_new_window_action = m_menu_list.m_dev_tools_menu->addAction("Open in New Window", this, &BrowserImpl::openDeveloperToolsWindow);

            open_in_new_window_action->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Q));

            parentMenu->addMenu(m_menu_list.m_dev_tools_menu.get());
        }
    }

    void BrowserImpl::addActionWithShortcut(QMenu *menu, const QString &actionName, const QKeySequence &shortcut, const std::function<void()>& slot)
    {
        if (menu) {
            QAction *action = menu->addAction(actionName, this, slot);
            action->setShortcut(shortcut);
        }
    }

    void BrowserImpl::adjustZoom(float delta)
    {
        auto currentZoom = m_view->page()->zoomFactor();
        m_view->page()->setZoomFactor(currentZoom + delta);
        zoomChanged(currentZoom);
    }

    void BrowserImpl::moveDeveloperToolsLeft()
    {
        if (!is_open_developer_tools_window) {
            if (!m_view->page()->devToolsPage()) {
                m_view->page()->setDevToolsPage(m_developer_tools_view->page());
            }

            m_developer_tools_view->show();
            m_main_splitter->setOrientation(Qt::Horizontal);
            m_main_splitter->insertWidget(0, m_developer_tools_view.get());
            m_main_splitter->setSizes({width() / 3, width() * 2 / 3});
            is_open_developer_tools_view = true;
        }
    }

    void BrowserImpl::moveDeveloperToolsRight()
    {
        if (!is_open_developer_tools_window) {
            if (!m_view->page()->devToolsPage()) {
                m_view->page()->setDevToolsPage(m_developer_tools_view->page());
            }

            m_developer_tools_view->show();
            m_main_splitter->setOrientation(Qt::Horizontal);
            m_main_splitter->insertWidget(1, m_developer_tools_view.get());
            m_main_splitter->setSizes({width() * 2 / 3, width() / 3});
            is_open_developer_tools_view = true;
        }
    }

    void BrowserImpl::moveDeveloperToolsTop()
    {
        if (!is_open_developer_tools_window) {
            if (!m_view->page()->devToolsPage()) {
                m_view->page()->setDevToolsPage(m_developer_tools_view->page());
            }

            m_developer_tools_view->show();
            m_main_splitter->setOrientation(Qt::Vertical);
            m_main_splitter->insertWidget(0, m_developer_tools_view.get());
            m_main_splitter->setSizes({height() / 3, height() * 2 / 3});
            is_open_developer_tools_view = true;
        }
    }

    void BrowserImpl::moveDeveloperToolsBottom()
    {
        if (!is_open_developer_tools_window) {
            if (!m_view->page()->devToolsPage()) {
                m_view->page()->setDevToolsPage(m_developer_tools_view->page());
            }

            m_developer_tools_view->show();
            m_main_splitter->setOrientation(Qt::Vertical);
            m_main_splitter->insertWidget(1, m_developer_tools_view.get());
            m_main_splitter->setSizes({height() * 2 / 3, height() / 3});
            is_open_developer_tools_view = true;
        }
    }

    void BrowserImpl::openDeveloperToolsWindow() 
    {
        if (!is_open_developer_tools_view) {
            m_view->page()->setDevToolsPage(m_developer_tools_window->page());
            m_developer_tools_window->resize(600, 600);
            m_developer_tools_window->show();
            m_developer_tools_window->raise();
            is_open_developer_tools_window = true;
        }
    }

    W_OBJECT_IMPL(BrowserImpl)
}