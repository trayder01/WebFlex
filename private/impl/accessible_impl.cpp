#include "webflex/impl/accessible_impl.hpp"
#include "webflex/core/utils.hpp"
#include "webflex/application.hpp"

namespace webflex::impl
{
    JsAccessibleImpl::JsAccessibleImpl(QObject *parent) : QObject(parent)
    {
        m_pool.setMaxThreadCount(qMax(webflex::Application::threadCount(), 4u));
    }

    void JsAccessibleImpl::callAsync(const QString &name, const QVariantList &args)
    {
        auto arguments = prepareArguments(args);

        {
            std::lock_guard<std::mutex> lock(m_return_members_mutex);
            auto returnIt = m_return_members.find(name);
            if (returnIt != m_return_members.end())
            {
                m_pool.start([this, name, returnIt, arguments = std::move(arguments)]{ 
                    asyncCallCompleted(name, webflex::utils::fromStdVariantToQvariant(returnIt->second.second(std::move(arguments)))); 
                });
                return;
            }
        }

        {
            std::lock_guard<std::mutex> lock(m_no_return_members_mutex);
            auto noReturnIt = m_no_return_members.find(name);
            if (noReturnIt != m_no_return_members.end())
            {
                m_pool.start([noReturnIt, arguments = std::move(arguments)]{ 
                    noReturnIt->second.second(std::move(arguments)); 
                });
                return;
            }
        }
    }

    QVariant JsAccessibleImpl::call(const QString &name, const QVariantList &args)
    {
        auto arguments = prepareArguments(args);

        {
            std::lock_guard<std::mutex> lock(m_return_members_mutex);
            auto returnIt = m_return_members.find(name);
            if (returnIt != m_return_members.end())
            {
                return webflex::utils::fromStdVariantToQvariant(returnIt->second.second(std::move(arguments)));
            }
        }

        {
            std::lock_guard<std::mutex> lock(m_no_return_members_mutex);
            auto noReturnIt = m_no_return_members.find(name);
            if (noReturnIt != m_no_return_members.end())
            {
                noReturnIt->second.second(std::move(arguments));
                return {};
            }
        }

        return {};
    }

    JsArguments JsAccessibleImpl::prepareArguments(const QVariantList &args)
    {
        JsArguments arguments;
        for (const auto &arg : args)
        {
            utils::fillArguments(arguments, arg);
        }
        return arguments;
    }

    W_OBJECT_IMPL(JsAccessibleImpl)
}