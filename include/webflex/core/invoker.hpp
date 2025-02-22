#ifndef WINVOKER_HPP
#define WINVOKER_HPP

#include <QVariantList>
#include <QString>
#include <QThreadPool>
#include <mutex>
#include <shared_mutex>

#include <unordered_map>

#include "wobjectcpp.h"
#include "../defines.hpp"

namespace webflex::core {
    class Invoker : public QObject
    {
        W_OBJECT(Invoker)
        using callback = std::function<QVariant(const QVariantList& args)>;
        using args = QVariantList;

    public:
        explicit Invoker(QObject* parent = nullptr);
        ~Invoker();

        void add(const QString& name, const callback& c);
        
        void remove(const QString& name);

        void callAsync(const QString& name, const args& arg);
        W_INVOKABLE(callAsync)

        QVariant call(const QString& name, const args& arg);
        W_INVOKABLE(call)

        void asyncCallCompleted(const QString& name, const QVariant& result)
        W_SIGNAL(asyncCallCompleted, name, result)

    private:
        std::unordered_map<QString, std::function<QVariant(const QVariantList& args)>> m_functions;
        QThreadPool m_pool;
        mutable std::shared_mutex m_shared_mutex;
    };
}

#endif // WINVOKER_HPP