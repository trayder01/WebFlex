#include "webflex/impl/accessible_impl.hpp"
#include "webflex/impl/browser_impl.hpp"
#include "webflex/core/wobjectimpl.h"
#include "webflex/impl/page_impl.hpp"
#include "webflex/application.hpp"
#include "webflex/core/dispatch.hpp"
#include "webflex/cookie_store.hpp"
#include "webflex/core/utils.hpp"
#include "webflex/accessible.hpp"
#include "webflex/navigation.hpp"
#include "webflex/browser.hpp"
#include "webflex/pdf.hpp"

#include <QWebEngineUrlScheme>
#include <QWebEngineSettings>
#include <QWebEngineScriptCollection>
#include <QWebEngineHistory>
#include <QWebEngineProfile>
#include <QNetworkCookie>
#include <QWebEngineCookieStore>
#include <QResizeEvent>
#include <QMenuBar>
#include <QFileInfo>
#include <QFile>

namespace webflex {
    std::unordered_set<std::string> Browser::m_register_schemes = {};

    Browser::Browser() : m_impl(std::make_unique<impl::BrowserImpl>())
    {
        QObject::connect(m_impl->view(), &QWebEngineView::loadFinished, [this](bool ok)
        {
            if (ok) { 
                m_dom_ready = true;
                if (!m_pending_scripts.empty()) {
                    for (const auto& script : m_pending_scripts) 
                    {
                        evaluate(script.first, script.second);
                    }
                    m_pending_scripts.clear();
                }
            }
        });

        initConnections();
    }

    Browser::~Browser() 
    {
        clearScripts();
        this->m_dom_ready = false;
        this->m_icon.clear();
        this->m_scripts_permanent.clear();
        this->m_register_schemes.clear();
        this->m_read_data.clear();
        this->m_embed_classes.clear();
    }

    std::shared_ptr<Browser> Browser::create() 
    {
        auto c = std::make_shared<Browser>();
        return std::move(c);
    }
    
    std::shared_ptr<Browser> Browser::create(const std::string& title) 
    {
        auto c = create();
        c->setTitle(title);
        return std::move(c);
    }

    std::shared_ptr<Browser> Browser::create(const std::string& title, const geometry::Size& size) 
    {
        auto c = create(title);
        c->resize(size);
        return std::move(c);
    }

    std::shared_ptr<Browser> Browser::create(const std::string& title, const geometry::Size& size, const geometry::Point& point) 
    {
        auto c = create(title, size);
        c->move(point);
        return std::move(c);
    }

    void Browser::inject(InjectOptions options) 
    {
        if (options.code.empty()) { return; }
        
        core::dispatch([this, options]()
        {
            W_ASSERT(m_impl && "m_impl is null in Browser::inject");

            static size_t script_next_id_{0}; 

            const auto view = m_impl->view();
            const auto script_id_ = "script_" + QString::number(script_next_id_++); 

            auto& scripts = view->page()->scripts();
            bool script_found = false;

            if (!m_script_set.empty()) {
                if (options.permanent && m_script_set.find(script_id_.toStdString()) != m_script_set.end())
                {
                    script_found = true;
                }
            }

            if (!script_found)
            {
                QWebEngineScript script;
                script.setInjectionPoint(static_cast<QWebEngineScript::InjectionPoint>(options.time));
                script.setWorldId(static_cast<QWebEngineScript::ScriptWorldId>(options.world));
                script.setSourceCode(QString::fromStdString(options.code));
                script.setName(script_id_);
                scripts.insert(script);

                if (options.permanent)
                {
                    m_script_set.insert(script_id_.toStdString());
                }

                m_scripts_permanent.push_back({options.permanent, script_id_.toStdString()});
            }
        });
    }

    void Browser::clearHttpCache() 
    {
        core::dispatch([this]()
        {
            W_ASSERT(m_impl && "m_impl is null in Browser::clearHttpCache");

            const auto view = m_impl->view();

            view->page()->profile()->clearHttpCache();
        });   
    }

    void Browser::execute(const std::string& script) 
    {
        if (script.empty()) { return; }

        core::dispatch([this, script = std::move(script)]()
        {
            W_ASSERT(m_impl && "m_impl is null in Browser::execute");

            const auto view = m_impl->view();

            if (!m_dom_ready) 
            {
                m_pending_scripts.emplace_back(std::make_pair(std::move(script), [](const core::js_any_t &){}));
                return;
            }

            view->page()->runJavaScript(QString::fromStdString(script));
        });
    }

    void Browser::evaluateImpl(const std::string& script, const std::function<void(const core::js_any_t &)> &resultCallback) 
    {
        if (script.empty()) { return; }

        core::dispatch([this, script = std::move(script), resultCallback = std::move(resultCallback)]()
        {
            W_ASSERT(m_impl && "m_impl is null in Browser::execute");

            const auto view = m_impl->view();

            if (!m_dom_ready) 
            {
                m_pending_scripts.emplace_back(std::make_pair(std::move(script), std::move(resultCallback)));
                return;
            }

            view->page()->runJavaScript(QString::fromStdString(script), [result_callback = std::move(resultCallback)](const QVariant& result)
            {
                if (!result_callback) { return; }
                result_callback(utils::convertQvariantToStdVariant(result));
            });
        });
    }

    void Browser::exposeImplNoReturn(const std::string &name, const std::function<void(const JsArguments &args)> &callback, Launch police) 
    {
        if (name.empty() || !callback) { return; }

        core::dispatch([this, name = std::move(name), callback = std::move(callback), police = std::move(police)]() 
        {
            W_ASSERT(m_impl && "m_impl is null in Browser::exposeImplNoReturn");

            m_impl->invoker()->add(QString::fromStdString(name), [this, callback = std::move(callback)](const QVariantList& args) 
            {
                if (!args.isEmpty()) {
                    JsArguments arguments;

                    for (const auto& arg : args) 
                    {
                        utils::fillArguments(arguments, arg);
                    }

                    callback(std::move(arguments));
                    return QVariant{};
                }

                callback({});
                return QVariant{};
            });

            std::ostringstream inject_script;
            inject_script << "(() => {\n"
                        << "    if (!window." << name << ") {\n"
                        << "        Object.defineProperty(window, '" << name << "', {\n"
                        << "            value: function(...args) {\n";

            if (police == Launch::Async) 
            {
                inject_script << "                return new Promise((resolve, reject) => {\n"
                            << "                    const callback = (name, value, error) => {\n"
                            << "                        window.invoker.asyncCallCompleted.disconnect(callback);\n"
                            << "                        if (error) reject(new Error(error));\n"
                            << "                        else resolve(value);\n"
                            << "                    };\n"
                            << "                    window.invoker.asyncCallCompleted.connect(callback);\n"
                            << "                    window.invoker.callAsync('" << name << "', args);\n"
                            << "                });\n";
            }
            else 
            {
                inject_script << "                return window.invoker.call('" << name << "', args);\n";
            }

            inject_script << "            },\n"
                        << "            writable: false,\n"
                        << "            configurable: false\n"
                        << "        });\n"
                        << "    }\n"
                        << "})();\n";

            inject({.code = inject_script.str(), .time = ExecutionTime::DocumentReady, .world = InjectWorld::Main});
        });
    }

    void Browser::exposeImplWithReturn(const std::string &name, const std::function<core::js_any_t(const JsArguments &args)> &callback, Launch police) 
    {
        if (name.empty() || !callback) { return; }

        core::dispatch([this, name, callback, police]()
        {
            W_ASSERT(m_impl && "m_impl is null in Browser::exposeImplWithReturn");

            m_impl->invoker()->add(QString::fromStdString(name), [this, callback = std::move(callback)](const QVariantList& args)
            {
                if (!args.isEmpty()) {
                    JsArguments arguments;
                    for (const auto& arg : args) 
                    {
                        utils::fillArguments(arguments, arg);
                    }
                    return utils::fromStdVariantToQvariant(callback(std::move(arguments)));
                }

                return utils::fromStdVariantToQvariant(callback({}));
            });

            std::ostringstream inject_script;
            inject_script << "(() => {\n"
                        << "    if (!window." << name << ") {\n"
                        << "        Object.defineProperty(window, '" << name << "', {\n"
                        << "            value: function(...args) {\n";

            if (police == Launch::Async) 
            {
                inject_script << "                return new Promise((resolve, reject) => {\n"
                            << "                    const callback = (name, value, error) => {\n"
                            << "                        window.invoker.asyncCallCompleted.disconnect(callback);\n"
                            << "                        if (error) reject(new Error(error));\n"
                            << "                        else resolve(value);\n"
                            << "                    };\n"
                            << "                    window.invoker.asyncCallCompleted.connect(callback);\n"
                            << "                    window.invoker.callAsync('" << name << "', args);\n"
                            << "                });\n";
            }
            else 
            {
                inject_script << "                return window.invoker.call('" << name << "', args);\n";
            }

            inject_script << "            },\n"
                        << "            writable: false,\n"
                        << "            configurable: false\n"
                        << "        });\n"
                        << "    }\n"
                        << "})();\n";

            inject({ .code = inject_script.str(), .time = ExecutionTime::DocumentReady, .world = InjectWorld::Main });
        });
    }
    
    void Browser::unexpose(const std::string& name) 
    {
        if (name.empty()) { return; }

        core::dispatch([this, name]()
        {
            W_ASSERT(m_impl && "m_impl is null in Browser::unexpose");
            m_impl->invoker()->remove(QString::fromStdString(name));
            clearScripts();
        });
    }

    void Browser::loadFile(const std::string& path) 
    {
        if (path.empty()) { return; }
        core::dispatch([this, path]()
        {
            W_ASSERT(m_impl && "m_impl is null in Browser::loadFile");

            const auto view = m_impl->view();
            
            QString qPath = QString::fromStdString(path);

            auto applicationDir = webflex::Application::getPath(webflex::PathKey::ApplicationDir);

            view->load((QFileInfo(qPath).isAbsolute() ? 
            QUrl::fromLocalFile(qPath) : 
            QUrl::fromLocalFile(!applicationDir.empty() ? QString::fromStdString(applicationDir) : QCoreApplication::applicationDirPath()  + "/" + qPath)));
        });
    }

    void Browser::loadHTML(const std::string& html) 
    {
        if (html.empty()) { return; }

        core::dispatch([this, html]()
        {
            W_ASSERT(m_impl && "m_impl is null in Browser::loadHTML");
            
            const auto view = m_impl->view();
            view->setHtml(QString::fromStdString(html));
        });
    }

    void Browser::setVisible(bool state) 
    {
        core::dispatch([this, state]()
        {
            W_ASSERT(m_impl && "m_impl is null in Browser::setVisible");
            m_impl->setVisible(state);
        });
    }

    void Browser::setMaximumSize(const geometry::Size& size) 
    {
        core::dispatch([this, size]()
        {
            m_maximum_size = size;
            
            W_ASSERT(m_impl && "m_impl is null in Browser::setMaximumSize");
            m_impl->setMaximumSize(size.width(), size.height());
        });
    }

    void Browser::setMaximumSize(int w, int h) 
    {
        setMaximumSize(geometry::Size(w, h));
    }
 
    geometry::Size Browser::maximumSize() const 
    {
        return m_maximum_size;
    }

    void Browser::setMinimumSize(const geometry::Size& size) 
    {
        core::dispatch([this, size]()
        {
            W_ASSERT(m_impl && "m_impl is null in Browser::setMinimumSize");
            m_minimum_size = size;
            m_impl->setMinimumSize(size.width(), size.height());
        });
    }
    
    void Browser::setMinimumSize(int w, int h) 
    {
        setMaximumSize(geometry::Size(w, h));
    }
    
    geometry::Size Browser::minimumSize() const 
    {
        return m_minimum_size;
    }

    void Browser::setMaximumWidth(int w) 
    {
        core::dispatch([this, w]()
        {
            W_ASSERT(m_impl && "m_impl is null in Browser::setMaximumWidth");
            m_impl->setMaximumWidth(w);
        });
    }

    void Browser::setMaximumHeight(int h) 
    {
        core::dispatch([this, h]()
        {
            W_ASSERT(m_impl && "m_impl is null in Browser::setMaximumHeight");
            m_impl->setMaximumHeight(h);
        });
    }

    void Browser::setMinimumWidth(int w) 
    {
        core::dispatch([this, w]()
        {
            W_ASSERT(m_impl && "m_impl is null in Browser::setMinimumWidth");
            m_impl->setMinimumWidth(w);
        });
    }

    void Browser::setMinimumHeight(int h) 
    {
        core::dispatch([this, h]()
        {
            W_ASSERT(m_impl && "m_impl is null in Browser::setMinimumHeight");
            m_impl->setMinimumHeight(h);
        });
    }

    bool Browser::isVisible() const 
    {
        return core::dispatch([this]()
        {
            W_ASSERT(m_impl && "m_impl is null in Browser::isVisible");
            return m_impl->isVisible();
        });
    }

    void Browser::clearScripts() 
    {
        core::dispatch([this]()
        {
            W_ASSERT(m_impl && "m_impl is null in Browser::clearScripts");

            const auto view = m_impl->view();

            auto& scripts = view->page()->scripts();

            for (const auto& script_perm : m_scripts_permanent) 
            {
                for (const auto& script : scripts.toList()) 
                {
                    if (script.name() == script_perm.second.c_str())
                    {
                        auto first = scripts.find(script.name()).first();

                        bool permament = script_perm.first; 

                        if (!permament) 
                        {
                            scripts.remove(first); 
                            break;
                        }
                    }
                }
            }
        });
    }

    void Browser::htmlText(const std::function<void(const std::string&)>& result, bool wait_dom_ready) 
    {
        core::dispatch([this, callback = std::move(result), wait_dom_ready]()
        {
            W_ASSERT(m_impl && "m_impl is null in Browser::html_text");

            const auto view = m_impl->view();

            auto toTextCallback = [callback](const QString& content)
            {
                callback(content.toStdString());
            };

            auto toTextErrorCallback = [callback](const QString& errorMessage)
            {
                callback(errorMessage.toStdString());
            };

            QObject::disconnect(view, &QWebEngineView::loadFinished, nullptr, nullptr);

            if (wait_dom_ready) {
                QObject::connect(view, &QWebEngineView::loadFinished, [this, view, toTextCallback, toTextErrorCallback](bool ok)
                {
                    if (ok) {
                        view->page()->toHtml(toTextCallback);
                    } else {
                        toTextErrorCallback("Page failed to load.");
                    }
                });
            }
            else 
            {
                view->page()->toHtml(toTextCallback);
            }
        });
    }

    void Browser::plainText(const std::function<void(const std::string&)>& result, bool wait_dom_ready) 
    {
        core::dispatch([this, callback = std::move(result), wait_dom_ready]()
        {
            W_ASSERT(m_impl && "m_impl is null in Browser::plain_text");

            const auto view = m_impl->view();

            auto toTextCallback = [callback](const QString& content)
            {
                callback(content.toStdString());
            };

            auto toTextErrorCallback = [callback](const QString& errorMessage)
            {
                callback(errorMessage.toStdString());
            };

            QObject::disconnect(view, &QWebEngineView::loadFinished, nullptr, nullptr);

            if (wait_dom_ready) {
                QObject::connect(view, &QWebEngineView::loadFinished, [view, toTextCallback, toTextErrorCallback](bool ok)
                {
                    if (ok) {
                        view->page()->toPlainText(toTextCallback);
                    } else {
                        toTextErrorCallback("Page failed to load.");
                    }
                });
            }
            else 
            {
                view->page()->toPlainText(toTextCallback);
            }
        });
    }
    
    void Browser::setIcon(const Icon& icon) 
    {
        core::dispatch([this, icon]()
        {
            W_ASSERT(m_impl && "m_impl is null in Browser::setIcon");

            m_icon = icon;

            m_impl->setWindowIcon(utils::convertIcon(icon));
        });
    }

    std::shared_ptr<Navigation>& Browser::navigation() 
    {
        if (!m_navigation) {
            m_navigation.reset(new Navigation(this));
        }

        return m_navigation;
    }

    std::shared_ptr<CookieStore>& Browser::cookies() 
    {
        if (!m_cookies) {
            m_cookies.reset(new CookieStore(this));
        }

        return m_cookies;
    }

    std::shared_ptr<Pdf>& Browser::pdf() 
    {
        if (!m_pdf) {
            m_pdf.reset(new Pdf(this));
        }

        return m_pdf;
    }

    Icon Browser::icon() const 
    {
        return core::dispatch([this]()
        {
            W_ASSERT(m_impl && "m_impl is null in Browser::icon");
            return m_icon;
        });
    }

    void Browser::setTitle(const std::string& title) 
    {
        core::dispatch([this, title]()
        {
            W_ASSERT(m_impl && "m_impl is null in Browser::set_title");
            m_impl->setWindowTitle(QString::fromStdString(title));
        });
    }

    void Browser::embedImpl(std::shared_ptr<JsAccessible> object, const std::string_view& className) 
    {
        if (m_embed_classes.find(className) != m_embed_classes.end()) { return; }

        core::dispatch([this, object, className]()
        {
            W_ASSERT(m_impl && "m_impl is null in Browser::embedImpl");

            std::string className_obj(className);
            
            object->impl()->setObjectName(QString::fromStdString(className_obj));

            m_impl->channel()->registerObject(className_obj.c_str(), object->impl().get());

            auto members = object->methods();
            auto fields = object->fields();

            std::ostringstream inject_script;
            inject_script << "(() => {\n"
                        << "    if (!window." << className_obj << ") {\n"
                        << "        Object.defineProperty(window, '" << className_obj << "', {\n"
                        << "            value: {},\n"
                        << "            writable: false,\n"
                        << "            configurable: false\n"
                        << "        });\n"
                        << "    }\n\n";

            for (const auto& field : fields) 
            {
                inject_script << "  Object.defineProperty(window." << className_obj << ", '" << field << "', {\n"
                                << "    get: async function() {\n"
                                << "        return await qchannel.objects." << className_obj << ".call('get__" << field << "', []);\n"
                                << "    },\n"
                                << "    set: function(newValue) {\n"
                                << "        qchannel.objects." << className_obj << ".call('set__" << field << "', [newValue]);\n"
                                << "    }\n"
                                << "  });\n";
            }           

            for (const auto& [member_name, police] : members) 
            {
                inject_script << "    Object.defineProperty(window." << className_obj << ", '" 
                            << member_name << "', {\n"
                            << "        value: function(...args) {\n";

                if (police == Launch::Async) 
                {
                    inject_script << "            return new Promise((resolve, reject) => {\n"
                                << "                const callback = (name, value, error) => {\n"
                                << "                    qchannel.objects." << className_obj 
                                << ".asyncCallCompleted.disconnect(callback);\n"
                                << "                    if (error) reject(new Error(error));\n"
                                << "                    else resolve(value);\n"
                                << "                };\n"
                                << "                qchannel.objects." << className_obj 
                                << ".asyncCallCompleted.connect(callback);\n"
                                << "                qchannel.objects." << className_obj 
                                << ".callAsync('" << member_name << "', args);\n"
                                << "            });\n";
                }
                else 
                {
                    inject_script << "            return qchannel.objects." << className_obj 
                                << ".call('" << member_name << "', args);\n";
                }

                inject_script << "        },\n"
                            << "        writable: false,\n"
                            << "        configurable: false\n"
                            << "    });\n";
            }

            inject_script << "})();\n";

            inject({.code = inject_script.str(), .time = ExecutionTime::DocumentReady, .world = InjectWorld::Main});

            m_embed_classes.insert(className);
        });
    }

    void Browser::registerScheme(const std::string& name) 
    {
        m_register_schemes.insert(name);

        QWebEngineUrlScheme scheme(QByteArray::fromStdString(name));
        scheme.setSyntax(QWebEngineUrlScheme::Syntax::HostAndPort); 
        scheme.setFlags(QWebEngineUrlScheme::Flag::SecureScheme | QWebEngineUrlScheme::Flag::LocalScheme | QWebEngineUrlScheme::Flag::FetchApiAllowed);
        QWebEngineUrlScheme::registerScheme(scheme);
    }

    void Browser::setBackgroundColor(const Color& color) 
    {
        m_background_color = color;

        core::dispatch([this, color]()
        {
            W_ASSERT(m_impl && "m_impl is null in Browser::remove_handle_scheme");

            const auto view = m_impl->view();    

            view->page()->setBackgroundColor(QColor(QString::fromStdString(m_background_color.name())));        
        });
    }

    Color Browser::backgroundColor() const 
    {
        return m_background_color;
    }

    std::string Browser::title() const 
    {
        return core::dispatch([this]()
        {
            W_ASSERT(m_impl && "m_impl is null in Browser::title");
            return m_impl->windowTitle().toStdString();
        });
    }

    void Browser::resize(int w, int h) 
    {
        core::dispatch([this, w, h]()
        {
            W_ASSERT(m_impl && "m_impl is null in Browser::resize");
            m_impl->resize(w, h);
        });
    }

    void Browser::resize(const geometry::Size& size) 
    {
        core::dispatch([this, size]()
        {
            resize(size.width(), size.height());
        });
    }

    void Browser::move(int x, int y) 
    {
        core::dispatch([this, x, y]()
        {
            W_ASSERT(m_impl && "m_impl is null in Browser::move");
            m_impl->move(x, y);
        });
    }

    void Browser::move(const geometry::Point& point) 
    {
        core::dispatch([this, point]()
        {
            move(point.x(), point.y());
        });
    }

    geometry::Size Browser::size() const 
    {
        return core::dispatch([this]()
        {
            W_ASSERT(m_impl && "m_impl is null in Browser::size");
            auto s = m_impl->size();
            return geometry::Size{s.width(), s.height()};
        });
    }

    geometry::Point Browser::position() const 
    {
        return core::dispatch([this]()
        {
            W_ASSERT(m_impl && "m_impl is null in Browser::position");
            auto s = m_impl->pos();
            return geometry::Point{s.x(), s.y()};
        });
    }

    void Browser::reload() 
    {
        core::dispatch([this]()
        {
            W_ASSERT(m_impl && "m_impl is null in Browser::reload");
            const auto view = m_impl->view();
            view->reload();
        });
    }
    
    void Browser::stop() 
    {
        core::dispatch([this]
        {
            W_ASSERT(m_impl && "m_impl is null in Browser::stop");
            const auto view = m_impl->view();
            view->stop();
        });
    }

    void Browser::setTheme(ThemeMode mode) 
    {
        m_theme_mode = mode;

        core::dispatch([this, mode]()
        {
            W_ASSERT(m_impl && "m_impl is null in Browser::setTheme");

            const auto view = m_impl->view();

            switch (mode)
            {
                case ThemeMode::System: 
                {
                    auto theme = utils::systemTheme();
                    if (theme.find("dark") != std::string::npos || theme.find("Dark") != std::string::npos) 
                    {
                        view->settings()->setAttribute(QWebEngineSettings::ForceDarkMode, true);
                    }
                    else if (theme.find("light") != std::string::npos || theme.find("Light") != std::string::npos) 
                    {
                        view->settings()->setAttribute(QWebEngineSettings::ForceDarkMode, false);
                    }
                    break;
                }
                case ThemeMode::Light: 
                {
                    view->settings()->setAttribute(QWebEngineSettings::ForceDarkMode, false);
                    break;
                }
                case ThemeMode::Dark: 
                {
                    view->settings()->setAttribute(QWebEngineSettings::ForceDarkMode, true);
                    break;
                }
            }
        });
    }

    ThemeMode Browser::theme() const 
    {
        return m_theme_mode;
    }

    int Browser::width() const 
    {
        return core::dispatch([this]()
        {
            W_ASSERT(m_impl && "m_impl is null in Browser::width");
            return m_impl->width();
        });
    }

    int Browser::height() const 
    {
        return core::dispatch([this]()
        {
            W_ASSERT(m_impl && "m_impl is null in Browser::height");
            return m_impl->height();
        });
    }

    int Browser::x() const 
    {
        return core::dispatch([this]()
        {
            W_ASSERT(m_impl && "m_impl is null in Browser::x");
            return m_impl->x();
        });
    }

    int Browser::y() const 
    {
        return core::dispatch([this]()
        {
            W_ASSERT(m_impl && "m_impl is null in Browser::y");
            return m_impl->y();
        });
    }

    void Browser::show(ShowMode mode) 
    {
        core::dispatch([this, mode]()
        {
            W_ASSERT(m_impl && "m_impl is null in Browser::show");
            switch (mode) 
            {
                case ShowMode::Normal: {
                    m_impl->show();
                    break;
                }
                case ShowMode::FullScreen: {
                    m_impl->showFullScreen();
                    break;
                }
                case ShowMode::Maximized: {
                    m_impl->showMaximized();
                    break;
                }
                case ShowMode::Minimized: {
                    m_impl->showMinimized();
                    break;
                }
            }

        });
    }

    void Browser::hide() 
    {
        core::dispatch([this]()
        {
            W_ASSERT(m_impl && "m_impl is null in Browser::hide");
            m_impl->hide();
        });
    }

    bool Browser::isHidden() 
    {
        return core::dispatch([this]()
        {
            W_ASSERT(m_impl && "m_impl is null in Browser::isHidden");
            return m_impl->isHidden();
        });
    }
    
    void Browser::close() 
    {
        core::dispatch([this]()
        {
            W_ASSERT(m_impl && "m_impl is null in Browser::close");
            m_impl->close();
        });
    }

    void Browser::initConnections() 
    {
        const auto view = m_impl->view();

        QObject::connect(m_impl.get(), &impl::BrowserImpl::resized, [this](int w, int h)
        {
            if (m_resized_signal.slot_count() > 0) 
            {
                m_resized_signal(w, h);
            }
        });

        QObject::connect(m_impl.get(), &impl::BrowserImpl::close_, [this]()
        {
            if (m_close_signal.slot_count() > 0) 
            {
                m_close_signal();
            }
        });

        QObject::connect(m_impl.get(), &impl::BrowserImpl::closed, [this]()
        {
            if (m_closed_signal.slot_count() > 0) 
            {
                m_closed_signal();
            }
        });

        QObject::connect(m_impl.get(), &impl::BrowserImpl::maximize, [this](bool state)
        {
            if (m_maximized_signal.slot_count() > 0) 
            {
                m_maximized_signal(state);
            }
        });

        QObject::connect(m_impl.get(), &impl::BrowserImpl::minimize, [this](bool state)
        {
            if (m_minimized_signal.slot_count() > 0) 
            {
                m_minimized_signal(state);
            }
        });

        QObject::connect(view, &QWebEngineView::loadFinished, [this](bool ok)
        {
            if (m_dom_ready_signal.slot_count() > 0) 
            {
                m_dom_ready_signal(ok);
            }
        });

        QObject::connect(view, &QWebEngineView::iconChanged, [this](const QIcon& icon)
        {
            if (m_favicon_changed_signal.slot_count() > 0) 
            {
                m_favicon_changed_signal(utils::convertIcon(icon));
            }
        });

        auto title_callback = [this](const QString& title)
        {
            if (m_title_changed_signal.slot_count() > 0) 
            {
                m_title_changed_signal(title.toStdString());
            }
        };

        QObject::connect(view, &QWebEngineView::titleChanged, title_callback);

        QObject::connect(view, &QWebEngineView::loadProgress, [this](int progress)
        {
            if (m_load_progress_signal.slot_count() > 0) 
            {
                m_load_progress_signal(progress);
            }
        });

        QObject::connect(view->page(), &QWebEnginePage::scrollPositionChanged, [this](const QPointF& pos)
        {
            if (m_scrolled_signal.slot_count() > 0) 
            {
                m_scrolled_signal(pos.x(), pos.y());
            }
        });

        QObject::connect(view->page(), &QWebEnginePage::loadStarted, [this]()
        {
            if (m_load_started_signal.slot_count() > 0) 
            {
                m_load_started_signal();
            }
        });

        QObject::connect(m_impl.get(), &impl::BrowserImpl::zoomChanged, [this](double zoom)
        {
            if (m_zoom_changed_signal.slot_count() > 0) 
            {
                m_zoom_changed_signal(zoom);
            }
        });

        QObject::connect(view->page(), &QWebEnginePage::urlChanged, [this](const QUrl& url)
        {
            if (m_navigated_signal.slot_count() > 0) 
            {
                m_navigated_signal(Url(url.toString().toStdString()));
            }
        });

        QObject::connect(view->page()->profile(), &QWebEngineProfile::clearHttpCacheCompleted, [this]()
        {
            if (m_clear_http_cache_signal.slot_count() > 0) 
            {
                m_clear_http_cache_signal();
            }
        });

        QObject::connect(view->page()->profile()->cookieStore(), &QWebEngineCookieStore::cookieAdded, [this](const QNetworkCookie &cookie) {
            if (m_cookie_added_signal.slot_count() > 0) {
                Cookie webCookie;
                webCookie.is_secure = cookie.isSecure();
                webCookie.is_http_only = cookie.isHttpOnly();
                webCookie.same_site_policy = (cookie.sameSitePolicy() == QNetworkCookie::SameSite::None) ? SameSite::None : SameSite::Default;
                webCookie.is_session_cookie = cookie.isSessionCookie();
                webCookie.domain = cookie.domain().toStdString();
                webCookie.path = cookie.path().toStdString();
                webCookie.name = cookie.name().toStdString();
                webCookie.value = ByteArray(cookie.value().toStdString());
                m_cookie_added_signal(webCookie);
            }
        });

        QObject::connect(view->page()->profile()->cookieStore(), &QWebEngineCookieStore::cookieRemoved, [this](const QNetworkCookie &cookie) {
            if (m_cookie_removed_signal.slot_count() > 0) {
                Cookie webCookie;
                webCookie.is_secure = cookie.isSecure();
                webCookie.is_http_only = cookie.isHttpOnly();
                webCookie.same_site_policy = (cookie.sameSitePolicy() == QNetworkCookie::SameSite::None) ? SameSite::None : SameSite::Default;
                webCookie.is_session_cookie = cookie.isSessionCookie();
                webCookie.domain = cookie.domain().toStdString();
                webCookie.path = cookie.path().toStdString();
                webCookie.name = cookie.name().toStdString();
                webCookie.value = ByteArray(cookie.value().toStdString());
                m_cookie_removed_signal(webCookie);
            }
        });
    }
}