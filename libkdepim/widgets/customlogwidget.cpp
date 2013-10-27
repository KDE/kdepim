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

#include "customlogwidget.h"
#include <QTextDocument>
#include <QPainter>
#include <QApplication>
#include <QAbstractTextDocumentLayout>

#include <KDebug>
using namespace KPIM;

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

  const QString content = QString::fromLatin1 (
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

QWidget  *LogItemDelegate::createEditor ( QWidget *, const QStyleOptionViewItem  &, const QModelIndex & ) const
{
  return 0;
}



CustomLogWidget::CustomLogWidget( QWidget * parent )
  :QListWidget( parent )
{
  LogItemDelegate *itemDelegate = new LogItemDelegate( this );
  setItemDelegate( itemDelegate );
}

CustomLogWidget::~CustomLogWidget()
{
}

void CustomLogWidget::addTitleLogEntry( const QString &log )
{
    QListWidgetItem* item =new QListWidgetItem(log);
    item->setForeground(Qt::black);
    QFont font = item->font();
    font.setBold(true);
    item->setFont(font);
    item->setData(ItemLogType, Title);
    addItem( item );
}

void CustomLogWidget::addInfoLogEntry( const QString &log )
{
  QListWidgetItem* item =new QListWidgetItem(log);
  item->setForeground(Qt::blue);
  item->setData(ItemLogType, Info);
  addItem( item );
}

void CustomLogWidget::addErrorLogEntry( const QString &log )
{
  QListWidgetItem* item =new QListWidgetItem(log);
  item->setForeground(Qt::red);
  item->setData(ItemLogType, Error);
  addItem( item );
}

QString CustomLogWidget::toHtml() const
{
    QString result = QLatin1String("<html>\n<body>\n");
    result += QLatin1String("<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">");
    for (int i=0; i < count(); ++i)  {
        QListWidgetItem *itemWidget = item(i);
        const QString itemText(itemWidget->text());
        QString logText;
        LogType type = static_cast<LogType>(itemWidget->data(CustomLogWidget::ItemLogType).toInt());
        switch(type) {
        case Title:
            logText = QString::fromUtf8( "<font color=%1>%2</font>" ).arg(QColor(Qt::black).name()).arg(itemText);
            break;
        case Error:
            logText = QString::fromUtf8( "<font color=%1>%2</font>" ).arg(QColor(Qt::red).name()).arg(itemText);
            break;
        case Info:
            logText = QString::fromUtf8( "<font color=%1>%2</font>" ).arg(QColor(Qt::green).name()).arg(itemText);
            break;
        default:
            kDebug()<<"LogType undefined"<<type;
            logText += itemWidget->text();
            break;
        }
        result += QLatin1String("<p>") + logText + QLatin1String("</p>");
    }
    result += QLatin1String( "</body>\n</html>\n" );
    return result;
}

QString CustomLogWidget::toPlainText() const
{
    QString result;
    for (int i=0; i < count(); ++i)  {
        result += item(i)->text() + QLatin1Char('\n');
    }
    return result;
}
