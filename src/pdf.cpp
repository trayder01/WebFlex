#include "webflex/impl/browser_impl.hpp"
#include "webflex/browser.hpp"
#include "webflex/pdf.hpp"

#include <QWebEngineView>
#include <QWebEnginePage>
#include <QPageLayout>
#include <QPageSize>
#include <QMarginsF>
#include <filesystem>
#include <functional>

namespace webflex {
    Pdf::Pdf() {}

    Pdf::~Pdf() {}

    Pdf::Pdf(webflex::Browser* browser) : browser_{browser}
    {
        if (!browser_ || !browser_->m_impl || !browser_->m_impl->centralWidget()) {
            W_THROW(std::runtime_error("Invalid browser or implementation"));
        }
    }

    void Pdf::print(const std::filesystem::path& path, PdfOptions options, bool wait_dom_ready)
    {
        if (!browser_ || !browser_->m_impl || !browser_->m_impl->centralWidget()) {
            W_THROW(std::runtime_error("Invalid browser or implementation"));
        }

        if (path.empty()) { return; }

        const auto view = browser_->m_impl->view();
        auto page = view->page();

        auto layout = QPageLayout(
            QPageSize(static_cast<QPageSize::PageSizeId>(options.page_size_id)),
            static_cast<QPageLayout::Orientation>(options.orientation),
            QMarginsF(options.margins.left(), options.margins.top(), options.margins.right(), options.margins.bottom())
        );

        if (wait_dom_ready) 
        {
            QObject::connect(page, &QWebEnginePage::loadFinished, [page, path, layout](bool ok)
            {
                if (ok) {
                    page->printToPdf(QString::fromStdString(path.string()), layout);
                }
            });
            return;
        }

        page->printToPdf(QString::fromStdString(path.string()), layout);
    }

    void Pdf::print(const std::function<void(const webflex::ByteArray&)>& data, PdfOptions options, bool wait_dom_ready)
    {
        if (!browser_ || !browser_->m_impl || !browser_->m_impl->centralWidget()) {
            W_THROW(std::runtime_error("Invalid browser or implementation"));
        }

        const auto view = browser_->m_impl->view();
        auto page = view->page();

        auto layout = QPageLayout(
            QPageSize(static_cast<QPageSize::PageSizeId>(options.page_size_id)),
            static_cast<QPageLayout::Orientation>(options.orientation),
            QMarginsF(options.margins.left(), options.margins.top(), options.margins.right(), options.margins.bottom())
        );

        if (wait_dom_ready) 
        {
            QObject::connect(page, &QWebEnginePage::loadFinished, [page, data, layout](bool ok)
            {
                if (ok) {
                    page->printToPdf([data](const QByteArray& pdfData) {
                        if (data) {
                            webflex::ByteArray wrapper;
                            wrapper.append(pdfData.toStdString());
                            data(wrapper);
                        }
                    }, layout);
                }
            });
            return;
        }

        page->printToPdf([data](const QByteArray& pdfData) {
            if (data) {
                webflex::ByteArray wrapper;
                wrapper.append(pdfData.toStdString());
                data(wrapper);
            }
        }, layout);
    }
} // namespace webflex
