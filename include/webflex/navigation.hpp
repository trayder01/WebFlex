#ifndef WNAVIGATION_HPP
#define WNAVIGATION_HPP

#include "url.hpp"
#include "navigation_entry.hpp"
#include <functional>

namespace webflex {
    class Browser;
    class Navigation 
    {
    public:
        Navigation();
        ~Navigation();

        void loadUrl(const Url& Url);

        void loadUrlAndWait(const Url& Url, uint64_t timeout);

        void goBack();

        bool canGoBack() const;

        void goForward();

        void reloadIgnoringCache() const;

        void reloadAndCheckForRepost() const;

        void reloadIgnoringCacheAndCheckForRepost() const;

        void reload() const;

        void goToIndex(int index);

        void stop() const;

        bool canGoForward() const;

        Url url() const;

        NavigationEntry entryAtIndex(int index);

        NavigationEntry backItem() const;

        NavigationEntry forwardItem() const;

        int entryCount() const;

    private:
        explicit Navigation(Browser* browser);

        friend class Browser;

    private:
        Browser* m_browser = nullptr;
        std::vector<Url> m_urls;
        Url m_url;
    };
}

#endif // WNAVIGATION_HPP