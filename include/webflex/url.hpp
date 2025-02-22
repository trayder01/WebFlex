#ifndef WURL_HPP
#define WURL_HPP

#include <string>
#include <map>
#include <filesystem>

namespace webflex
{
    class Url
    {
    public:
        explicit Url(const std::string &Url = "");

        Url(const char* Url);

        Url(const Url& other);

        Url(Url&& other) noexcept;

        Url& operator=(const Url& other);

        Url& operator=(Url&& other) noexcept;

        bool operator==(const Url& other) const;
        bool operator!=(const Url& other) const;

        static Url fromLocalFile(const std::filesystem::path& path); 

        void setUrl(const std::string &Url);
        std::string toString() const;

        std::string protocol() const;
        std::string host() const;
        std::string path() const;
        std::string query() const;

        void setProtocol(const std::string &protocol);
        void setHost(const std::string &host);
        void setPath(const std::string &path);

        void addQueryParameter(const std::string &key, const std::string &value);
        void removeQueryParameter(const std::string &key);
        std::string queryParameter(const std::string &key) const;

        friend std::ostream& operator<<(std::ostream& os, const Url& other) 
        {
            os << std::string("Url(\"") << other.toString() << std::string("\")");
            return os;
        }

    private:
        std::string protocol_;
        std::string host_;
        std::string path_;
        std::map<std::string, std::string> queryParams_;
    };
}

#endif