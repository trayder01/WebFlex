#include "webflex/impl/browser_impl.hpp"
#include "webflex/core/dispatch.hpp"
#include "webflex/navigation.hpp"
#include "webflex/browser.hpp"
#include <QWebEngineHistory>
#include <QWebEngineView>
#include <QTimer>

namespace webflex {
    Navigation::Navigation() {}

    Navigation::Navigation(webflex::Browser* browser) : m_browser{browser} 
    {
    }

    Navigation::~Navigation() 
    {
        m_browser = nullptr;
        m_urls.clear();
    }

    void Navigation::loadUrlAndWait(const webflex::Url& Url, uint64_t timeout) 
    {
        QTimer::singleShot(timeout, [this, Url]()
        {
            loadUrl(Url);
        });
    }

    void Navigation::loadUrl(const webflex::Url& url) 
    {
        core::dispatch([this, url]()
        {
            W_ASSERT(m_browser && m_browser->m_impl && "m_impl is null in Navigation::loadUrl");

            m_url = url;

            const auto view = m_browser->m_impl->view();
            
            view->setUrl(QUrl(QString::fromStdString(m_url.toString())));
            
            m_urls.push_back(url);
        });
    }

    void Navigation::goBack() 
    {
        core::dispatch([this]()
        {
            W_ASSERT(m_browser && m_browser->m_impl && "m_impl is null in Navigation::goBack");

            const auto view = m_browser->m_impl->view();

            view->back();
        });
    }

    bool Navigation::canGoBack() const 
    {
        return core::dispatch([this]()
        {
            W_ASSERT(m_browser && m_browser->m_impl && "m_impl is null in Navigation::canGoBack");

            const auto view = m_browser->m_impl->view();

            return view->history()->canGoBack();
        });        
    }

    void Navigation::reload() const 
    {
        core::dispatch([this]()
        {
            W_ASSERT(m_browser && m_browser->m_impl && "m_impl is null in Navigation::reload");

            const auto view = m_browser->m_impl->view();

            view->reload();
        });
    }

    void Navigation::reloadIgnoringCache() const 
    {
        core::dispatch([this]()
        {
            W_ASSERT(m_browser && m_browser->m_impl && "m_impl is null in Navigation::reload");

            const auto view = m_browser->m_impl->view();

            view->page()->triggerAction(QWebEnginePage::WebAction::ReloadAndBypassCache, true);
        });
    }

    void Navigation::reloadAndCheckForRepost() const 
    {
        core::dispatch([this]()
        {
            W_ASSERT(m_browser && m_browser->m_impl && "m_impl is null in Navigation::reload");

            const auto view = m_browser->m_impl->view();

            auto page = view->page();

            if (page->action(QWebEnginePage::WebAction::Reload)->isEnabled()) 
            {
                view->page()->triggerAction(QWebEnginePage::WebAction::ReloadAndBypassCache, true);
            }
        });
    }

    void Navigation::reloadIgnoringCacheAndCheckForRepost() const 
    {
        core::dispatch([this]()
        {
            W_ASSERT(m_browser && m_browser->m_impl && "m_impl is null in Navigation::reload");

            const auto view = m_browser->m_impl->view();

            auto page = view->page();

            if (page->action(QWebEnginePage::WebAction::ReloadAndBypassCache)->isEnabled()) 
            {
                page->triggerAction(QWebEnginePage::WebAction::ReloadAndBypassCache);
            }
            else if (page->action(QWebEnginePage::WebAction::Reload)->isEnabled()) 
            {
                page->triggerAction(QWebEnginePage::WebAction::Reload);
            }
        });
    }

    void Navigation::stop() const 
    {
        core::dispatch([this]()
        {
            W_ASSERT(m_browser && m_browser->m_impl && "m_impl is null in Navigation::stop");

            const auto view = m_browser->m_impl->view();

            view->stop();
        });
    }

    void Navigation::goForward() 
    {
        core::dispatch([this]()
        {
            W_ASSERT(m_browser && m_browser->m_impl && "m_impl is null in Navigation::goForward");

            const auto view = m_browser->m_impl->view();

            view->forward();
        });
    }

    NavigationEntry Navigation::forwardItem() const 
    {
        return core::dispatch([this]()
        {
            W_ASSERT(m_browser && m_browser->m_impl && "m_impl is null in Navigation::forwardItem");

            const auto view = m_browser->m_impl->view(); 
          
            auto history = view->history();
            auto forwardItem = history->forwardItem();

            NavigationEntry entry;
            entry.m_title = forwardItem.title().toStdString();
            entry.m_url.setUrl(forwardItem.url().toString().toStdString());
            entry.m_is_valid = forwardItem.isValid();
            entry.m_original_url.setUrl(forwardItem.originalUrl().toString().toStdString());
            entry.m_icon_url.setUrl(forwardItem.iconUrl().toString().toStdString());
            entry.m_last_visited = forwardItem.lastVisited().toString().toStdString();

            return entry;                
        });
    }

    int Navigation::entryCount() const 
    {
        return core::dispatch([this]()
        {
            W_ASSERT(m_browser && m_browser->m_impl && "m_impl is null in Navigation::backItem");

            const auto view = m_browser->m_impl->view(); 
            return view->history()->count();             
        });
    }

    NavigationEntry Navigation::backItem() const 
    {
        return core::dispatch([this]()
        {
            W_ASSERT(m_browser && m_browser->m_impl && "m_impl is null in Navigation::backItem");

            const auto view = m_browser->m_impl->view(); 
            
            auto history = view->history();
            auto backItem = history->backItem();

            NavigationEntry entry;
            entry.m_title = backItem.title().toStdString();
            entry.m_url.setUrl(backItem.url().toString().toStdString());
            entry.m_is_valid = backItem.isValid();
            entry.m_original_url.setUrl(backItem.originalUrl().toString().toStdString());
            entry.m_icon_url.setUrl(backItem.iconUrl().toString().toStdString());
            entry.m_last_visited = backItem.lastVisited().toString().toStdString();

            return entry;                
        });
    }

    NavigationEntry Navigation::entryAtIndex(int index) 
    {
        return core::dispatch([this, index]()
        {
            W_ASSERT(m_browser && m_browser->m_impl && "m_impl is null in Navigation::itemAt");

            const auto view = m_browser->m_impl->view();

            auto history = view->history();
            auto qentry = history->itemAt(index);

            NavigationEntry entry;
            entry.m_title = qentry.title().toStdString();
            entry.m_url.setUrl(qentry.url().toString().toStdString());
            entry.m_is_valid = qentry.isValid();
            entry.m_original_url.setUrl(qentry.originalUrl().toString().toStdString());
            entry.m_icon_url.setUrl(qentry.iconUrl().toString().toStdString());
            entry.m_last_visited = qentry.lastVisited().toString().toStdString();

            return entry;
        });
    }

    void Navigation::goToIndex(int index) 
    {
        if (index >= 0 && index < m_urls.size()) {
            loadUrl(m_urls.at(index));
        }
    }

    bool Navigation::canGoForward() const 
    {
        return core::dispatch([this]()
        {
            W_ASSERT(m_browser && m_browser->m_impl && "m_impl is null in Navigation::canGoForward");

            const auto view = m_browser->m_impl->view();
            return view->history()->canGoForward();
        });
    }
    
    webflex::Url Navigation::url() const 
    {
        return m_url;
    }
}