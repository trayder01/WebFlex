#include "webflex/navigation_entry.hpp"

namespace webflex
{
    NavigationEntry::NavigationEntry(const NavigationEntry &other) : m_icon_url{other.m_icon_url}, m_is_valid{other.m_is_valid}, m_last_visited{other.m_last_visited}, m_original_url{other.m_original_url}, m_title{other.m_title}, m_url{other.m_url}
    {
    }

    NavigationEntry::NavigationEntry(NavigationEntry &&other) : m_icon_url{std::move(other.m_icon_url)}, m_is_valid{std::move(other.m_is_valid)}, m_last_visited{std::move(other.m_last_visited)}, m_original_url{std::move(other.m_original_url)}, m_title{std::move(other.m_title)}, m_url{std::move(other.m_url)}
    {
    }

    NavigationEntry &NavigationEntry::operator=(const NavigationEntry &other)
    {
        if (this != &other)
        {
            this->m_icon_url = other.m_icon_url;
            this->m_is_valid = other.m_is_valid;
            this->m_last_visited = other.m_last_visited;
            this->m_original_url = other.m_original_url;
            this->m_title = other.m_title;
            this->m_url = other.m_url;
        }

        return *this;
    }

    NavigationEntry &NavigationEntry::operator=(NavigationEntry &&other)
    {
        if (this != &other)
        {
            this->m_icon_url = std::move(other.m_icon_url);
            this->m_is_valid = std::move(other.m_is_valid);
            this->m_last_visited = std::move(other.m_last_visited);
            this->m_original_url = std::move(other.m_original_url);
            this->m_title = std::move(other.m_title);
            this->m_url = std::move(other.m_url);
        }

        return *this;
    }

    NavigationEntry::~NavigationEntry() {}

    webflex::Url NavigationEntry::originalUrl() const
    {
        return this->m_original_url;
    }

    webflex::Url NavigationEntry::url() const
    {
        return this->m_url;
    }

    std::string NavigationEntry::title() const
    {
        return this->m_title;
    }

    std::string NavigationEntry::lastVisited() const
    {
        return this->m_last_visited;
    }

    webflex::Url NavigationEntry::iconUrl() const
    {
        return this->m_icon_url;
    }

    bool NavigationEntry::isValid() const
    {
        return this->m_is_valid;
    }
}