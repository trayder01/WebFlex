#ifndef NAGIGATION_ENTRY_HPP
#define NAGIGATION_ENTRY_HPP

#include "url.hpp"

namespace webflex
{
    class NavigationEntry
    {
    public:
        NavigationEntry() = default;
        NavigationEntry(const NavigationEntry &other);
        NavigationEntry(NavigationEntry &&other);
        NavigationEntry &operator=(const NavigationEntry &other);
        NavigationEntry &operator=(NavigationEntry &&other);
        ~NavigationEntry();

        Url originalUrl() const;

        Url url() const;

        std::string title() const;

        std::string lastVisited() const;

        Url iconUrl() const;

        bool isValid() const;

        friend class Navigation;

        friend std::ostream &operator<<(std::ostream &os, const NavigationEntry &entry)
        {
            os << "NavigationEntry {"
               << "\n  Title: " << entry.m_title
               << "\n  Url: " << entry.m_url
               << "\n  Original Url: " << entry.m_original_url
               << "\n  Icon Url: " << entry.m_icon_url
               << "\n  Last Visited: " << entry.m_last_visited
               << "\n  Is Valid: " << (entry.m_is_valid ? "true" : "false")
               << "\n}";
            return os;
        }

    private:
        Url m_original_url;
        Url m_url;
        std::string m_title;
        Url m_icon_url;
        bool m_is_valid;
        std::string m_last_visited;
    };
}

#endif // NAGIGATION_ENTRY_HPP
