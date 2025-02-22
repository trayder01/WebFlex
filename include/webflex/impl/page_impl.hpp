#ifndef WPAGEIMPL_HPP
#define WPAGEIMPL_HPP

#include <QWebEnginePage>
#include "../core/wobjectcpp.h"

W_REGISTER_ARGTYPE(QUrl)
W_REGISTER_ARGTYPE(QString*)
W_REGISTER_ARGTYPE(QWebEnginePage::JavaScriptConsoleMessageLevel)

namespace webflex::impl {
    class PageImpl : public QWebEnginePage
    {
        W_OBJECT(PageImpl)
    public:
        explicit PageImpl(QObject *parent = nullptr);
        explicit PageImpl(QWebEngineProfile *profile, QObject *parent = nullptr);

        void alertChanged(const QUrl &securityOrigin, const QString &msg)
        W_SIGNAL(alertChanged, securityOrigin, msg)

        void promptChanged(const QUrl &securityOrigin, const QString &msg, const QString &defaultValue, QString *result, bool ok)
        W_SIGNAL(promptChanged, securityOrigin, msg, defaultValue, result, ok)

        void confirmChanged(const QUrl &securityOrigin, const QString &msg, bool ok)
        W_SIGNAL(confirmChanged, securityOrigin, msg, ok)

        void javaScriptConsoleMessageChanged(JavaScriptConsoleMessageLevel level, const QString& msg, int lineNumber, const QString& sourceId)
        W_SIGNAL(javaScriptConsoleMessageChanged, level, msg, lineNumber, sourceId)

    protected:
        void javaScriptAlert(const QUrl &securityOrigin, const QString &msg) override;

        bool javaScriptConfirm(const QUrl &securityOrigin, const QString &msg) override;
        
        bool javaScriptPrompt(const QUrl &securityOrigin, const QString &msg, const QString &defaultValue, QString *result) override;
        
        void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString &message, int lineNumber, const QString &sourceID) override;
    };
}

#endif // WPAGEIMPL_HPP