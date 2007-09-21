#include "knoteprinter.h"

#include <QAbstractTextDocumentLayout>
#include <QPainter>
#include <QTextDocument>
#include <QTextDocumentFragment>

#include <kcal/journal.h>

#include <klocale.h>
#include <kprinter.h>


KNotePrinter::KNotePrinter()
{
}

void KNotePrinter::setDefaultFont( const QFont &font )
{
  m_defaultFont = font;
}

QFont KNotePrinter::defaultFont() const
{
  return m_defaultFont;
}

void KNotePrinter::doPrint( const QString &htmlText,
                            const QString &dialogCaption ) const
{
  KPrinter printer( true, QPrinter::HighResolution );
  //printer.setFullPage( true );  //disabled, causes asymmetric margins
  if ( !printer.setup( 0, dialogCaption ) ) {
    return;
  }
  
  const int margin = 30; //pt     //set to 40 when setFullPage() works again
  int marginX = margin * printer.logicalDpiX() / 72;
  int marginY = margin * printer.logicalDpiY() / 72;
  
  QRect typeArea( marginX, marginY,
                  printer.width() - marginX * 2,
                  printer.height() - marginY * 2 );
  
  QTextDocument textDoc;
  textDoc.setHtml( htmlText );
  textDoc.documentLayout()->setPaintDevice( &printer );
  textDoc.setPageSize( typeArea.size() );
  textDoc.setDefaultFont( m_defaultFont );
  
  QPainter painter( &printer );
  QRect clip( typeArea );
  painter.translate( marginX, marginY );
  clip.translate( -marginX, -marginY );
  
  for ( int page = 1; page <= textDoc.pageCount(); page++ ) {
    textDoc.drawContents( &painter, clip );
    clip.translate( 0, typeArea.height() );
    painter.translate( 0, -typeArea.height() );
    
    painter.setFont( m_defaultFont );
    painter.drawText(
      clip.right() - painter.fontMetrics().width( QString::number( page ) ),
      clip.bottom() + painter.fontMetrics().ascent() + 5,
      QString::number( page ) );
    
    if ( page < textDoc.pageCount() ) {
      printer.newPage();
    }
  }
}

void KNotePrinter::printNote( const QString &name,
                              const QString &htmlText ) const
{
  QString dialogCaption = i18n( "Print %1", name );
  doPrint( htmlText, dialogCaption );
}

void KNotePrinter::printNotes( const QList<KCal::Journal *>& journals ) const
{
  if ( journals.isEmpty() ) {
    return;
  }
  
  QString htmlText;
  
  QListIterator<KCal::Journal *> it( journals );
  while ( it.hasNext() ) {
    KCal::Journal *j = it.next();
    htmlText += "<h2>" + j->summary() + "</h2>";
    
    //### ensureHtmlText() is a hack.
    //Possible solution:
    //port most parts of KNotes to HTML (aka rich) text - it is easy enough to
    //pass plaintext as rich text (Qt:convertFromPlainText()) and the way back
    //should be doable, too.
    //Gold star solution: cleanly separate the two types - just never
    //pass a chunk of text without explicit type. Probably not worth it.
    
    htmlText += ensureHtmlText( j->description() );
    
    if ( it.hasNext() ) {
      htmlText += "<hr />";
    }
  }
  
  QString dialogCaption = i18np( "Print Note", "Print %1 notes",
                                 journals.count() );
  doPrint( htmlText, dialogCaption );
  
#if 0
  //### This should work more reliably but it doesn't work at all.
  
  QTextDocument ret;
  ret.setDefaultFont( m_defaultFont );
  QTextCursor retC( &ret );
  QTextDocument separator;
  separator.setHtml( "<html><body><br /><hr /><br /></body></html>" );
  QTextDocument header;
  QTextDocument note;
  
  QListIterator<KCal::Journal *> it( journals );
  while ( it.hasNext() ) {
    KCal::Journal *j = it.next();
    header.setHtml( "<html><body><h2>" + j->summary() + "</h2></body></html>" );
    retC.insertFragment( QTextDocumentFragment( &header ) );
    
    note.setHtml( ensureHtmlText( j->description() ) );
    retC.insertFragment( QTextDocumentFragment( &note ) );
    
    if ( it.hasNext() ) {
      retC.insertFragment( QTextDocumentFragment( &separator ) );
    }
  }
  
  QString dialogCaption = i18np( "Print Note", "Print %1 notes",
                                 journals.count() );
  doPrint( ret.toHtml(), dialogCaption );
#endif
}

inline QString KNotePrinter::ensureHtmlText( const QString &maybeRichText )
                                             const
{
  if ( Qt::mightBeRichText( maybeRichText ) ) {
    return maybeRichText; //... now probablyRichText
  } else {
    return Qt::convertFromPlainText( maybeRichText );
  }
}

