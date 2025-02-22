#include "webflex/browser.hpp"
#include "webflex/cookie_store.hpp"
#include "webflex/core/dispatch.hpp"

#include "webflex/impl/browser_impl.hpp"

QT_BEGIN_NAMESPACE
#include <QWebEngineCookieStore>
QT_END_NAMESPACE

#define GET_COOKIE W_ASSERT(m_browser && "Browser pointer is null!"); auto cookieStore = m_browser->m_impl->view()->page()->profile()->cookieStore();

namespace webflex {
    CookieStore::CookieStore(Browser* browser) : m_browser(browser)
    {
        GET_COOKIE

        QObject::connect(cookieStore, &QWebEngineCookieStore::cookieAdded, [&](const QNetworkCookie &cookie) {
            Cookie webCookie;
            webCookie.is_secure = cookie.isSecure();
            webCookie.is_http_only = cookie.isHttpOnly();
            webCookie.same_site_policy = (cookie.sameSitePolicy() == QNetworkCookie::SameSite::None) ? SameSite::None : SameSite::Default;
            webCookie.is_session_cookie = cookie.isSessionCookie();
            webCookie.domain = cookie.domain().toStdString();
            webCookie.path = cookie.path().toStdString();
            webCookie.name = cookie.name().toStdString();
            webCookie.value = ByteArray(cookie.value().toStdString());

            m_cookie_list.push_back(webCookie);
        });
    }

    CookieStore::~CookieStore() 
    {
        m_browser = nullptr;
        m_cookie_list.clear();
    }

    std::vector<Cookie> CookieStore::list() const 
    {
        return core::dispatch([this]()
        {
            GET_COOKIE
            
            cookieStore->loadAllCookies();

            return m_cookie_list;
        });
    }

    std::vector<Cookie> CookieStore::list(const std::string& url) const 
    {
        return core::dispatch([this, url]()
        {
            GET_COOKIE

            std::vector<Cookie> cookies;
            QUrl qUrl(QString::fromStdString(url));

            for (const auto& value : m_cookie_list) 
            {
                if (QString::fromStdString(value.domain) == qUrl.host()) {
                    cookies.push_back(value);
                }
            }

            cookieStore->loadAllCookies();
           
            return cookies;
        });
    }

    void CookieStore::deleteAll() 
    {
        core::dispatch([this]()
        {
            GET_COOKIE
            cookieStore->deleteAllCookies();
        });
    }

    void CookieStore::deleteCookie(const Cookie& cookie) 
    {
        core::dispatch([this, &cookie]()
        {
            GET_COOKIE

            QNetworkCookie qCookie;
            qCookie.setDomain(QString::fromStdString(cookie.domain));
            qCookie.setPath(QString::fromStdString(cookie.path));
            qCookie.setName(QString::fromStdString(cookie.name).toUtf8());
            qCookie.setValue(QString::fromStdString(cookie.value.toString()).toUtf8());
        
            cookieStore->deleteCookie(qCookie);
        });
    }

    void CookieStore::setCookie(const Cookie &cookie) 
    {
        core::dispatch([this, &cookie]()
        {
            GET_COOKIE

            QNetworkCookie qCookie;
            qCookie.setSecure(cookie.is_secure);
            qCookie.setHttpOnly(cookie.is_http_only);
            qCookie.setDomain(QString::fromStdString(cookie.domain));
            qCookie.setPath(QString::fromStdString(cookie.path));
            qCookie.setName(QString::fromStdString(cookie.name).toUtf8());
            qCookie.setValue(QString::fromStdString(cookie.value.toString()).toUtf8());
            qCookie.setSameSitePolicy(cookie.same_site_policy == SameSite::None ? QNetworkCookie::SameSite::None : QNetworkCookie::SameSite::Default);

            cookieStore->setCookie(qCookie);
        });
    }

    void CookieStore::persist() 
    {
        core::dispatch([this]()
        {
            GET_COOKIE
            cookieStore->loadAllCookies();
        });
    }

    void CookieStore::loadAllCookies() 
    {
        core::dispatch([this]()
        {
            GET_COOKIE
            cookieStore->loadAllCookies();
        });
    }
}