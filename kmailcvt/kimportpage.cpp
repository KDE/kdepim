/***************************************************************************
                          kimportpage.cpp  -  description
                             -------------------
    begin                : Fri Jan 17 2003
    copyright            : (C) 2003 by Laurence Anderso
    email                : l.d.anderson@warwick.ac.uk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kimportpage.h"

#include <kapplication.h>
#include <kstandarddirs.h>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QPainter>

LogItemDelegate::LogItemDelegate( QObject *parent )
  : QStyledItemDelegate( parent )
{
}

LogItemDelegate::~LogItemDelegate()
{
}

QTextDocument* LogItemDelegate::document ( const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return 0;
  QTextDocument *document = new QTextDocument ( 0 );
  document->setDocumentMargin( 1 );
  const QColor textColor = index.data( Qt::ForegroundRole ).value<QColor>();
  QStyleOptionViewItemV4 option4 = option;
  QStyledItemDelegate::initStyleOption( &option4, index );

  QString text = option4.text;

  QString content = QString::fromLatin1 (
                          "<html style=\"color:%1\">"
                          "<body> %2" ).arg ( textColor.name().toUpper() ).arg( text )
                      + QLatin1String ( "</table></body></html>" );

  document->setHtml ( content );
  
  return document;
}


void LogItemDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return;
  QTextDocument *doc = document ( option, index );
  if ( !doc )
    return;
  doc->setTextWidth( option.rect.width() );
  painter->setRenderHint ( QPainter::Antialiasing );
  
  QPen pen = painter->pen();

  QStyleOptionViewItemV4 opt ( option );
  opt.showDecorationSelected = true;
  QApplication::style()->drawPrimitive ( QStyle::PE_PanelItemViewItem, &opt, painter ); 
  painter->save();
  painter->translate ( option.rect.topLeft() );
  
  doc->drawContents ( painter );
  
  painter->restore();
  painter->setPen( pen );

  delete doc;
}

QSize LogItemDelegate::sizeHint ( const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return QSize ( 0, 0 );

  QTextDocument *doc = document ( option, index );
  if ( !doc )
    return QSize ( 0, 0 );

  const QSize size = doc->documentLayout()->documentSize().toSize();
  delete doc;

  return size;
}

QWidget  * LogItemDelegate::createEditor ( QWidget *, const QStyleOptionViewItem  &, const QModelIndex & ) const
{
  return 0;
}


KImportPage::KImportPage(QWidget *parent ) : KImportPageDlg(parent) {

	mIntroSidebar->setPixmap(KStandardDirs::locate("data", "kmailcvt/pics/step1.png"));
        LogItemDelegate *itemDelegate = new LogItemDelegate( _log );
        _log->setItemDelegate( itemDelegate );

}

KImportPage::~KImportPage() {
}

#include "kimportpage.moc"

