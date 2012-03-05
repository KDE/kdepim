/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "importlogwidget.h"
#include <QTextDocument>
#include <QPainter>
#include <QApplication>
#include <QAbstractTextDocumentLayout>
using namespace MailImporter;

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



ImportLogWidget::ImportLogWidget( QWidget * parent )
  :QListWidget( parent )
{
  LogItemDelegate *itemDelegate = new LogItemDelegate( this );
  setItemDelegate( itemDelegate );
}

ImportLogWidget::~ImportLogWidget()
{
}
