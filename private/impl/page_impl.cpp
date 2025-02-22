#include "webflex/impl/page_impl.hpp"
#include "webflex/core/wobjectimpl.h"
#include <QMessageBox>
#include <QInputDialog>

namespace webflex::impl
{
    PageImpl::PageImpl(QObject *parent) : QWebEnginePage(parent)
    {
    }

    PageImpl::PageImpl(QWebEngineProfile *profile, QObject *parent) : QWebEnginePage(profile, parent)
    {
    }

    void PageImpl::javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString &message, int lineNumber, const QString &sourceID)
    {
        javaScriptConsoleMessageChanged(level, message, lineNumber, sourceID);
        qDebug().noquote() << "js: " << message;
        QWebEnginePage::javaScriptConsoleMessage(level, message, lineNumber, sourceID);
    }

    void PageImpl::javaScriptAlert(const QUrl &securityOrigin, const QString &msg)
    {
        QMessageBox::information(nullptr, "JavaScript Alert", msg);

        alertChanged(securityOrigin, msg);
    }

    bool PageImpl::javaScriptConfirm(const QUrl &securityOrigin, const QString &msg)
    {
        QMessageBox::StandardButton reply = QMessageBox::question(nullptr, "JavaScript Confirm", msg,
            QMessageBox::Yes | QMessageBox::No);
        
        bool ok = (reply == QMessageBox::Yes);
        confirmChanged(securityOrigin, msg, ok);
        return ok;
    }

    bool PageImpl::javaScriptPrompt(const QUrl &securityOrigin, const QString &msg, const QString &defaultValue, QString *result)
    {
        bool ok = false;
        QString text = QInputDialog::getText(nullptr, "JavaScript Prompt", msg, QLineEdit::Normal, defaultValue, &ok);

        if (ok) {
            *result = text;
        }
        promptChanged(securityOrigin, msg, defaultValue, result, ok);
        return ok;
    }

    W_OBJECT_IMPL(PageImpl)
}
