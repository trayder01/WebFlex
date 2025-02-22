#include "webflex/core/invoker.hpp"
#include "webflex/core/wobjectimpl.h"
#include "webflex/application.hpp"
#include <QPointer>
#include <QDebug>

namespace webflex::core 
{
    Invoker::Invoker(QObject* parent) : QObject(parent) 
    {
        setObjectName("invoker");
        m_pool.setMaxThreadCount(qMax(webflex::Application::threadCount(), 4U));
    }

    Invoker::~Invoker() 
    {
        std::lock_guard<std::shared_mutex> locker(m_shared_mutex); 
        m_functions.clear();
    }

    void Invoker::remove(const QString& name) 
    {
        if (m_functions.find(name) == m_functions.end()) { 
            qDebug() << "Function" << name << "not found in Invoker.";
            return; 
        }
        m_functions.erase(name);
    }

    void Invoker::add(const QString& name, const callback& c) 
    {
        if (m_functions.find(name) != m_functions.end()) {
            qDebug() << "Function" << name << "already exists in Invoker.";
            return;
        }
        m_functions[name] = c;
    }

    void Invoker::callAsync(const QString& name, const args& arg) 
    {
        callback handler;
        
        {  
            std::shared_lock<std::shared_mutex> locker(m_shared_mutex); 
            auto it = m_functions.find(name);
            if (it == m_functions.end()) 
            {
                qDebug() << "Function" << name << "not found in Invoker.";
                return;
            }
            handler = it->second;  
        }

        QPointer<Invoker> self(this);
        m_pool.start([self, name, handler = std::move(handler), arg = std::move(arg)]()
        {
            if (self) {
                self->asyncCallCompleted(name, handler(arg));
            }
        });
    }

    QVariant Invoker::call(const QString& name, const args& arg) 
    {
        std::shared_lock<std::shared_mutex> locker(m_shared_mutex);
        auto it = m_functions.find(name);
        if (it == m_functions.end()) 
        {
            qDebug() << "Function" << name << "not found in Invoker.";
            return QVariant{};
        }
        return std::invoke(it->second, arg);
    }

    W_OBJECT_IMPL(Invoker)
}
