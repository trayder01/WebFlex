#ifndef COOKIE_STORE_HPP
#define COOKIE_STORE_HPP

#include <memory>
#include <string>
#include <vector>
#include "byte_array.hpp"
#include "core/signal.hpp"

namespace webflex {
    class Browser;

    enum class SameSite {
        Default,
        None,
        Lax,
        Strict
    };

    struct Cookie 
    {
        bool is_secure;
        bool is_http_only;
        SameSite same_site_policy;
        bool is_session_cookie;
        std::string domain;
        std::string path;
        std::string name;
        ByteArray value;
    };

    class CookieStore 
    {
    public:
        std::vector<Cookie> list() const;

        std::vector<Cookie> list(const std::string& url) const;

        void deleteAll();

        void deleteCookie(const Cookie& cookie);

        void setCookie(const Cookie &cookie);

        void persist();

        void loadAllCookies();

        ~CookieStore();
        
    private:
        explicit CookieStore(Browser* browser);

    private:
        Browser* m_browser = nullptr;

        friend class Browser;

    private:
        std::vector<Cookie> m_cookie_list;
    };
}

#endif // COOKIE_STORE_HPP