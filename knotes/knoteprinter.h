#ifndef KNOTEPRINTER_H
#define KNOTEPRINTER_H

#include <tqfont.h>
#include <tqpalette.h>
#include <tqstring.h>

class QMimeSourceFactory;
class QStyleSheet;
template <class T> class QValueList;
class KPrinter;

namespace KCal {
    class Journal;
}

class KNotePrinter {
public:

    KNotePrinter();

    void printNote( const TQString& name,
                    const TQString& content ) const;

    void printNotes( const TQValueList<KCal::Journal*>& journals ) const;

    void setFont( const TQFont& font );
    TQFont font() const;

    void setColorGroup( const TQColorGroup& colorGroup );
    TQColorGroup colorGroup() const;

    void setStyleSheet( TQStyleSheet* styleSheet );
    TQStyleSheet* styleSheet() const;

    void setContext( const TQString& context );
    TQString context() const;

    void setMimeSourceFactory( TQMimeSourceFactory* factory );
    TQMimeSourceFactory* mimeSourceFactory() const;

private:
    void doPrint( KPrinter& printer, TQPainter& painter,
                  const TQString& content ) const;

    TQColorGroup m_colorGroup;
    TQFont m_font;
    TQStyleSheet* m_styleSheet;
    TQMimeSourceFactory* m_mimeSourceFactory;
    TQString m_context;
};

#endif // KNOTEPRINTER
