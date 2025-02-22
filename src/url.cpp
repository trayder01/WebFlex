#include "webflex/url.hpp"
#include <QUrl>
#include <QUrlQuery>
#include <QDebug>

namespace webflex
{
    Url::Url(const char *Url) : webflex::Url(std::string(Url)) {}

    Url::Url(const std::string &Url)
    {
        if (!Url.empty())
        {
            setUrl(Url);
        }
    }

    Url::Url(const Url &other)
        : protocol_(other.protocol_),
          host_(other.host_),
          path_(other.path_),
          queryParams_(other.queryParams_) {}

    Url::Url(Url &&other) noexcept
        : protocol_(std::move(other.protocol_)),
          host_(std::move(other.host_)),
          path_(std::move(other.path_)),
          queryParams_(std::move(other.queryParams_)) {}

    Url &Url::operator=(const Url &other)
    {
        if (this != &other)
        {
            protocol_ = other.protocol_;
            host_ = other.host_;
            path_ = other.path_;
            queryParams_ = other.queryParams_;
        }
        return *this;
    }

    Url &Url::operator=(Url &&other) noexcept
    {
        if (this != &other)
        {
            protocol_ = std::move(other.protocol_);
            host_ = std::move(other.host_);
            path_ = std::move(other.path_);
            queryParams_ = std::move(other.queryParams_);
        }
        return *this;
    }

    bool Url::operator==(const Url &other) const
    {
        return protocol_ == other.protocol_ &&
               host_ == other.host_ &&
               path_ == other.path_ &&
               queryParams_ == other.queryParams_;
    }

    bool Url::operator!=(const Url &other) const
    {
        return !(*this == other);
    }

    Url Url::fromLocalFile(const std::filesystem::path &path)
    {
        std::string Url = "file:///" + path.string();
        webflex::Url result(Url);
        return result;
    }

    void Url::setUrl(const std::string &Url)
    {
        QUrl qurl(QString::fromStdString(Url));
        protocol_ = qurl.scheme().toStdString();
        host_ = qurl.host().toStdString();
        path_ = qurl.path().toStdString();

        queryParams_.clear();
        QUrlQuery query(qurl);
        for (const auto &pair : query.queryItems())
        {
            queryParams_[pair.first.toStdString()] = pair.second.toStdString();
        }
    }

    std::string Url::toString() const
    {
        QUrl qurl;
        qurl.setScheme(QString::fromStdString(protocol_));
        qurl.setHost(QString::fromStdString(host_));
        qurl.setPath(QString::fromStdString(path_));

        QUrlQuery query;
        for (const auto &param : queryParams_)
        {
            query.addQueryItem(QString::fromStdString(param.first), QString::fromStdString(param.second));
        }
        qurl.setQuery(query);

        return qurl.toString().toStdString();
    }

    std::string Url::protocol() const
    {
        return protocol_;
    }

    std::string Url::host() const
    {
        return host_;
    }

    std::string Url::path() const
    {
        return path_;
    }

    std::string Url::query() const
    {
        QUrlQuery query;
        for (const auto &param : queryParams_)
        {
            query.addQueryItem(QString::fromStdString(param.first), QString::fromStdString(param.second));
        }
        return query.toString().toStdString();
    }

    void Url::setProtocol(const std::string &protocol)
    {
        protocol_ = protocol;
    }

    void Url::setHost(const std::string &host)
    {
        host_ = host;
    }

    void Url::setPath(const std::string &path)
    {
        path_ = path;
    }

    void Url::addQueryParameter(const std::string &key, const std::string &value)
    {
        queryParams_[key] = value;
    }

    void Url::removeQueryParameter(const std::string &key)
    {
        queryParams_.erase(key);
    }

    std::string Url::queryParameter(const std::string &key) const
    {
        auto it = queryParams_.find(key);
        if (it != queryParams_.end())
        {
            return it->second;
        }
        return {};
    }
}
