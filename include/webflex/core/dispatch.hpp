#pragma once

#include <QApplication>
#include <QThread>
#include <QMetaObject>
#include <QFuture>
#include <QPromise>
#include <type_traits>

namespace webflex::core
{
    template <typename Callable>
    auto dispatch(Callable &&callable)
    {
        using return_type = std::invoke_result_t<Callable>;

        if (qApp->thread() == QThread::currentThread())
        {
            if constexpr (std::is_void_v<return_type>)
            {
                std::invoke(std::forward<Callable>(callable));
            }
            else
            {
                return std::invoke(std::forward<Callable>(callable));
            }
        }
        else
        {
            if constexpr (std::is_void_v<return_type>)
            {
                bool is_call = QMetaObject::invokeMethod(
                    qApp,
                    [callable = std::forward<Callable>(callable)]() mutable
                    { callable(); },
                    Qt::QueuedConnection);

                if (!is_call)
                {
                    qWarning() << "Failed to invoke callable in the main thread.";
                }
            }
            else
            {
                std::shared_ptr<std::promise<return_type>> promise = std::make_shared<std::promise<return_type>>();
                auto future = promise->get_future();

                bool is_call = QMetaObject::invokeMethod(
                    qApp,
                    [callable = std::forward<Callable>(callable), promise]() mutable
                    {
                        try
                        {
                            promise->set_value(std::invoke(callable));
                        }
                        catch (...)
                        {
                            promise->set_exception(std::current_exception());
                        }
                    },
                    Qt::QueuedConnection);

                if (!is_call)
                {
                    qWarning() << "Failed to invoke callable in the main thread.";
                }

                return future.get();
            }
        }
    }
}
