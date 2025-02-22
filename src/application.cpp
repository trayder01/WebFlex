#include "webflex/application.hpp"
#include "webflex/defines.hpp"
#include <QApplication>
#include <QThread>

namespace webflex::impl {
    struct ApplicationImpl 
    {
        std::unique_ptr<QApplication> app_;
    };
}

namespace webflex 
{
    size_t Application::m_thread_count = 0;
    std::unordered_map<PathKey, std::string> Application::m_paths = {};

    Application::Application(AppOptions options) 
    {
        if (!impl_) {
            impl_ = std::make_unique<impl::ApplicationImpl>();

            W_ASSERT(impl_ && "impl_ is null in Application::Application");

            id_ = options.id.empty() ? "main" : options.id;

            if (!options.browser_flag.empty()) {
                std::string join_flags{};

                for (const auto& flag : options.browser_flag) {
                    join_flags += " " + flag;   
                }

                qputenv("QTWEBENGINE_CHROMIUM_FLAGS", QByteArrayView(join_flags));
            }

            argv_.clear();
            
            argv_.assign({const_cast<char*>(options.id.c_str()), nullptr});
            
            argc_ = static_cast<int>(argv_.size()) - 1;

            impl_->app_ = std::make_unique<QApplication>(argc_, argv_.data());

            if (options.thread_count != 0) {
                m_thread_count = options.thread_count;
            }
            else {
                m_thread_count = std::thread::hardware_concurrency();
            }
        }
    }
    
    Application::~Application() 
    {
        if (impl_ && impl_->app_) {
            impl_->app_.reset();
            impl_.reset();
        }
    }

    std::unique_ptr<Application> Application::init(AppOptions options) 
    {
        return std::make_unique<Application>(options);
    }

    const std::string& Application::getPath(PathKey key) 
    {
        return m_paths[key];
    }

    void Application::setPath(PathKey key, const std::string& path) 
    {
        m_paths[key] = path;
    }

    std::string Application::id() const 
    {
        return id_;
    }

    void Application::quit() const 
    {
        QApplication::quit();
    }

    int Application::run() const 
    {
        return QApplication::exec();
    }

    bool Application::threadSafe() 
    {
        return qApp->thread() == QThread::currentThread();
    }
}