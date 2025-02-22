#ifndef ACCESSIBLE_IMPL_HPP
#define ACCESSIBLE_IMPL_HPP

#include "webflex/core/wobjectcpp.h"
#include "webflex/core/wobjectimpl.h"
#include "webflex/defines.hpp"
#include "webflex/js_args.hpp"
#include "webflex/core/js_types.hpp"

#include <QObject>
#include <QThreadPool>

namespace webflex::impl {
    class JsAccessibleImpl : public QObject
    {
        W_OBJECT(JsAccessibleImpl)
    public:
        explicit JsAccessibleImpl(QObject* parent = nullptr);

        void callAsync(const QString& name, const QVariantList& arg);
        W_INVOKABLE(callAsync)
        
        QVariant call(const QString& name, const QVariantList& args);
        W_INVOKABLE(call)

        void asyncCallCompleted(const QString& name, const QVariant& value)
        W_SIGNAL(asyncCallCompleted, name, value)

        template <typename T>
        void add(const QString& name, const std::pair<Launch, T>& pair) 
        {
            if constexpr (std::is_same_v<std::invoke_result_t<T, const JsArguments&>, void>) {
                m_no_return_members[name] = std::move(pair);
            } else {
                m_return_members[name] = std::move(pair);
            }
        }

        JsArguments prepareArguments(const QVariantList& args);

    private:
        std::unordered_map<QString, std::pair<Launch, std::function<webflex::core::js_any_t(const JsArguments&)>>> m_return_members;
        std::unordered_map<QString, std::pair<Launch, std::function<void(const JsArguments&)>>> m_no_return_members;
        QThreadPool m_pool;
        mutable std::mutex m_return_members_mutex;  
        mutable std::mutex m_no_return_members_mutex;
    };
}

#endif // ACCESSIBLE_IMPL_HPP