#ifndef DEVTOOLS_VIEW_HPP
#define DEVTOOLS_VIEW_HPP

#include <QMainWindow>
#include <QWebEngineView>
#include <QShortcut>

#include "../core/wobjectcpp.h"

namespace webflex
{
    namespace impl
    {
        class DevToolsView : public QWebEngineView
        {
            W_OBJECT(DevToolsView)

        public:
            explicit DevToolsView(QWidget *parent = nullptr);

            ~DevToolsView();

            void closed()
            W_SIGNAL(closed)

        private:
            void setupShortcuts();
            void zoomIn(); W_SLOT(zoomIn)
            void zoomOut(); W_SLOT(zoomOut)
            void resetZoom(); W_SLOT(resetZoom)

        protected:
            void closeEvent(QCloseEvent* event) override;

        private:
            QScopedPointer<QShortcut> zoomInShortcutPlus;
            QScopedPointer<QShortcut> zoomInShortcutEqual;
            QScopedPointer<QShortcut> zoomOutShortcut;
            QScopedPointer<QShortcut> resetZoomShortcut;
        };
    } // namespace impl
} // namespace webflex

#endif // DEVTOOLS_VIEW_HPP
