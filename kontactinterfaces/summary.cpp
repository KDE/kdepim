/*
  This file is part of the KDE Kontact Plugin Interface Library.

  Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2003 Daniel Molkentin <molkentin@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "summary.h"

#include <QImage>
#include <QFont>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>

#include <KGlobalSettings>
#include <KHBox>
#include <KIconLoader>
#include <KDialog>

using namespace Kontact;

//@cond PRIVATE
namespace Kontact {
class SummaryMimeData : public QMimeData
{
  public:
    virtual bool hasFormat( const QString &format ) const
    {
      if ( format == "application/x-kontact-summary" ) {
        return true;
      }
      return false;
    }
};
}
//@endcond

//@cond PRIVATE
class Summary::Private
{
  public:
    KStatusBar *mStatusBar;
    QPoint mDragStartPoint;
};
//@endcond

Summary::Summary( QWidget *parent )
  : QWidget( parent ), d( new Private )
{
  setFont( KGlobalSettings::generalFont() );
  setAcceptDrops( true );
}

Summary::~Summary()
{
  delete d;
}

int Summary::summaryHeight() const
{
  return 1;
}

QWidget *Summary::createHeader( QWidget *parent, const QString &iconname, const QString &heading )
{
  setStyleSheet( "KHBox {"
                    "border: 0px;"
                    "font: bold large;"
                    "padding: 2px;"
                    "background: palette(window);"
                    "color: palette(windowtext);"
                 "}"
                 "KHBox > QLabel { font: bold larger; } " );

  KHBox *hbox = new KHBox( parent );

  QLabel *label = new QLabel( hbox );
  label->setPixmap( KIconLoader::global()->loadIcon( iconname, KIconLoader::Toolbar ) );

  label->setFixedSize( label->sizeHint() );
  label->setAcceptDrops( true );

  label = new QLabel( heading, hbox );
  label->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
  label->setIndent( KDialog::spacingHint() );

  hbox->setMaximumHeight( hbox->minimumSizeHint().height() );

  return hbox;
}

QStringList Summary::configModules() const
{
  return QStringList();
}

void Summary::updateSummary( bool force )
{
  Q_UNUSED( force );
}

void Summary::mousePressEvent( QMouseEvent *event )
{
  d->mDragStartPoint = event->pos();

  QWidget::mousePressEvent( event );
}

void Summary::mouseMoveEvent( QMouseEvent *event )
{
  if ( ( event->buttons() & Qt::LeftButton ) &&
       ( event->pos() - d->mDragStartPoint ).manhattanLength() > 4 ) {

    QDrag *drag = new QDrag( this );
    drag->setMimeData( new SummaryMimeData() );
    drag->setObjectName( "SummaryWidgetDrag" );

    QPixmap pm = QPixmap::grabWidget( this );
    if ( pm.width() > 300 ) {
      pm = QPixmap::fromImage(
        pm.toImage().scaled( 300, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation ) );
    }

    QPainter painter;
    painter.begin( &pm );
    painter.setPen( QPalette::AlternateBase );
    painter.drawRect( 0, 0, pm.width(), pm.height() );
    painter.end();
    drag->setPixmap( pm );
    drag->start( Qt::MoveAction );
  } else {
    QWidget::mouseMoveEvent( event );
  }
}

void Summary::dragEnterEvent( QDragEnterEvent *event )
{
  if ( event->mimeData()->hasFormat( "application/x-kontact-summary" ) ) {
    event->acceptProposedAction();
  }
}

void Summary::dropEvent( QDropEvent *event )
{
  int alignment = ( event->pos().y() < ( height() / 2 ) ? Qt::AlignTop : Qt::AlignBottom );
  emit summaryWidgetDropped( this, event->source(), alignment );
}

#include "summary.moc"
