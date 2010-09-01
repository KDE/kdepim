#include "knoteprinter.h"

#include <libkcal/journal.h>

#include <klocale.h>
#include <kprinter.h>
#include <kdebug.h>
#include <tqfont.h>
#include <tqpaintdevicemetrics.h>
#include <tqpainter.h>
#include <tqrect.h>
#include <tqsimplerichtext.h>
#include <tqstring.h>

KNotePrinter::KNotePrinter() : m_styleSheet( 0 ), m_mimeSourceFactory( 0 )
{
}

void KNotePrinter::setContext( const TQString& context )
{
    m_context = context;
}

TQString KNotePrinter::context() const
{
    return m_context;
}

void KNotePrinter::setMimeSourceFactory( TQMimeSourceFactory* factory )
{
    m_mimeSourceFactory = factory;
}

TQMimeSourceFactory* KNotePrinter::mimeSourceFactory() const
{
    return m_mimeSourceFactory;
}

void KNotePrinter::setFont( const TQFont& font )
{
    m_font = font;
}

TQFont KNotePrinter::font() const
{
    return m_font;
}

void KNotePrinter::setColorGroup( const TQColorGroup& colorGroup )
{
    m_colorGroup = colorGroup;
}

TQColorGroup KNotePrinter::colorGroup() const
{
    return m_colorGroup;
}

void KNotePrinter::setStyleSheet( TQStyleSheet* styleSheet )
{
    m_styleSheet = styleSheet;
}

TQStyleSheet* KNotePrinter::styleSheet() const
{
    return m_styleSheet;
}

void KNotePrinter::doPrint( KPrinter& printer, TQPainter& painter,
                            const TQString& content ) const
{
    const int margin = 40;  // pt

    TQPaintDeviceMetrics metrics( painter.device() );
    int marginX = margin * metrics.logicalDpiX() / 72;
    int marginY = margin * metrics.logicalDpiY() / 72;

    TQRect body( marginX, marginY,
            metrics.width() - marginX * 2,
            metrics.height() - marginY * 2 );

    kdDebug()<<" content :"<<content<<endl;
    kdDebug()<<" m_styleSheet :"<<m_styleSheet<<endl;
    //kdDebug()<<" m_font :"<<m_font;
    TQSimpleRichText text( content, m_font, m_context,
            m_styleSheet, m_mimeSourceFactory,
            body.height() /*, linkColor, linkUnderline? */ );

    text.setWidth( &painter, body.width() );
    TQRect view( body );

    int page = 1;

    for (;;)
    {
        text.draw( &painter, body.left(), body.top(), view, m_colorGroup );
        view.moveBy( 0, body.height() );
        painter.translate( 0, -body.height() );

        // page numbers
        painter.setFont( m_font );
        painter.drawText(
                view.right() - painter.fontMetrics().width( TQString::number( page ) ),
                view.bottom() + painter.fontMetrics().ascent() + 5, TQString::number( page )
                );

        if ( view.top() >= text.height() )
            break;

        printer.newPage();
        page++;
    }
}

void KNotePrinter::printNote( const TQString& name, const TQString& content ) const
{
    KPrinter printer;
    printer.setFullPage( true );

    if ( !printer.setup( 0, i18n("Print %1").arg(name) ) )
        return;
    TQPainter painter;
    painter.begin( &printer );
    doPrint( printer, painter, content );
    painter.end();
}

void KNotePrinter::printNotes( const TQValueList<KCal::Journal*>& journals ) const
{
    if ( journals.isEmpty() )
        return;

    KPrinter printer;
    printer.setFullPage( true );

    if ( !printer.setup( 0, i18n("Print Note", "Print %n notes", journals.count() ) ) )
        return;

    TQPainter painter;
    painter.begin( &printer );
    TQString content;
    TQValueListConstIterator<KCal::Journal*> it( journals.constBegin() );
    TQValueListConstIterator<KCal::Journal*> end( journals.constEnd() );
    while ( it != end ) {
        KCal::Journal *j = *it;
        it++;
        content += "<h2>" + j->summary() + "</h2>";
        content += j->description();
        if ( it != end )
            content += "<hr>";
    }
    doPrint( printer, painter, content );
    painter.end();
}


