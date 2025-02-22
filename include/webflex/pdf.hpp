#ifndef WPDF_HPP
#define WPDF_HPP

#include <filesystem>
#include <functional>
#include "byte_array.hpp"
#include "geometry/margins.hpp"

namespace webflex {
    class Browser;
    enum class PdfOrientation {
        Portrait,
        Landscape
    };

    enum class PdfPageSizeId {
        Letter, Legal, Executive, A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10,
        B0, B1, B2, B3, B4, B5, B6, B7, B8, B9, B10, C5E, Comm10E, DLE, Folio,
        Ledger, Tabloid, Custom,

        A3Extra, A4Extra, A4Plus, A4Small, A5Extra, B5Extra,
        JisB0, JisB1, JisB2, JisB3, JisB4, JisB5, JisB6, JisB7, JisB8, JisB9, JisB10,

        AnsiC, AnsiD, AnsiE, LegalExtra, LetterExtra, LetterPlus, LetterSmall,
        TabloidExtra, ArchA, ArchB, ArchC, ArchD, ArchE,

        Imperial7x9, Imperial8x10, Imperial9x11, Imperial9x12, Imperial10x11,
        Imperial10x13, Imperial10x14, Imperial12x11, Imperial15x11,

        ExecutiveStandard, Note, Quarto, Statement, SuperA, SuperB,
        Postcard, DoublePostcard, Prc16K, Prc32K, Prc32KBig,

        FanFoldUs, FanFoldGerman, FanFoldGermanLegal,
        EnvelopeB4, EnvelopeB5, EnvelopeB6, EnvelopeC0, EnvelopeC1, EnvelopeC2,
        EnvelopeC3, EnvelopeC4, EnvelopeC6, EnvelopeC65, EnvelopeC7, Envelope9,

        Envelope11, Envelope12, Envelope14, EnvelopeMonarch, EnvelopePersonal,
        EnvelopeChou3, EnvelopeChou4, EnvelopeInvite, EnvelopeItalian,
        EnvelopeKaku2, EnvelopeKaku3, EnvelopePrc1, EnvelopePrc2, EnvelopePrc3,
        EnvelopePrc4, EnvelopePrc5, EnvelopePrc6, EnvelopePrc7, EnvelopePrc8,
        EnvelopePrc9, EnvelopePrc10, EnvelopeYou4,

        LastPageSize = EnvelopeYou4,

        AnsiA = Letter, AnsiB = Ledger, EnvelopeC5 = C5E,
        EnvelopeDl = DLE, Envelope10 = Comm10E
    };

    struct PdfOptions {
        geometry::margins margins = {0, 0, 0, 0};
        PdfOrientation orientation = PdfOrientation::Portrait; 
        PdfPageSizeId page_size_id = PdfPageSizeId::A4;
    };

    class Pdf 
    {
    public:
        Pdf();
        ~Pdf();

        void print(const std::filesystem::path& path, PdfOptions options = {}, bool wait_dom_ready = false);
        void print(const std::function<void(const webflex::ByteArray&)>& data, PdfOptions options = {}, bool wait_dom_ready = false);

    private:
        Pdf(webflex::Browser* browser);
        
        friend class Browser;

    private:
        Browser* browser_ = nullptr;
    };
}

#endif // WPDF_HPP