/*
  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2007 Bruno Virlet <bruno@virlet.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/
#include "timelabels.h"
#include "agenda.h"
#include "prefs.h"
#include "timelabelszone.h"
#include "timescaleconfigdialog.h"

#include <KIcon>
#include <KLocalizedString>
#include <KGlobal>
#include <KTimeZone>

#include <QFrame>
#include <QMenu>
#include <QPainter>
#include <QPointer>
#include <KLocale>

//QT5 static const KCatalogLoader loader( QLatin1String("timezones4") );

using namespace EventViews;

TimeLabels::TimeLabels( const KDateTime::Spec &spec, int rows,
                        TimeLabelsZone *parent, Qt::WindowFlags f )
  : QFrame( parent, f )
{
  mTimeLabelsZone = parent;
  mSpec = spec;
  mRows = rows;
  mMiniWidth = 0;

  mCellHeight = mTimeLabelsZone->preferences()->hourSize() * 4;

  setBackgroundRole( QPalette::Background );

  mMousePos = new QFrame( this );
  mMousePos->setLineWidth( 1 );
  mMousePos->setFrameStyle( QFrame::HLine | QFrame::Plain );
  mMousePos->setFixedSize( width(), 1 );
#ifdef KDEPIM_MOBILE_UI
  mMousePos->hide();
#endif
  colorMousePos();
  mAgenda = 0;

  if ( mSpec.isValid() ) {
    setToolTip( i18n( "Timezone:" ) + i18n( mSpec.timeZone().name().toUtf8() ) );
  }

  setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );

  updateConfig();
 }

void TimeLabels::mousePosChanged( const QPoint &pos )
{
  colorMousePos();
  mMousePos->move( 0, pos.y() );

  // The repaint somehow prevents that the red line leaves a black artifact when
  // moved down. It's not a full solution, though.
  repaint();
}

void TimeLabels::showMousePos()
{
  // touch screen have no mouse position
#ifndef KDEPIM_MOBILE_UI
  mMousePos->show();
#endif
}

void TimeLabels::hideMousePos()
{
  mMousePos->hide();
}

void TimeLabels::colorMousePos()
{
  QPalette pal;
  pal.setColor( QPalette::Window, // for Oxygen
                mTimeLabelsZone->preferences()->agendaMarcusBainsLineLineColor() );
  pal.setColor( QPalette::WindowText, // for Plastique
                mTimeLabelsZone->preferences()->agendaMarcusBainsLineLineColor() );
  mMousePos->setPalette( pal );
}

void TimeLabels::setCellHeight( double height )
{
  if ( mCellHeight != height ) {
    mCellHeight = height;
    updateGeometry();
  }
}

QSize TimeLabels::minimumSizeHint() const
{
  QSize sh = QFrame::sizeHint();
  sh.setWidth( mMiniWidth );
  return sh;
}

/** updates widget's internal state */
void TimeLabels::updateConfig()
{
  setFont( mTimeLabelsZone->preferences()->agendaTimeLabelsFont() );

  QString test = QLatin1String("20");
  if ( KLocale::global()->use12Clock() ) {
    test = QLatin1String("12");
  }
  mMiniWidth = fontMetrics().width( test );
  if ( KLocale::global()->use12Clock() ) {
    test = QLatin1String("pm");
  } else {
    test = QLatin1String("00");
  }
  QFont sFont = font();
  sFont.setPointSize( sFont.pointSize() / 2 );
  QFontMetrics fmS( sFont );
  mMiniWidth += fmS.width( test ) + frameWidth() * 2 + 4 ;

  /** Can happen if all resources are disabled */
  if ( !mAgenda ) {
     return;
  }

  // update HourSize
  mCellHeight = mTimeLabelsZone->preferences()->hourSize() * 4;
  // If the agenda is zoomed out so that more than 24 would be shown,
  // the agenda only shows 24 hours, so we need to take the cell height
  // from the agenda, which is larger than the configured one!
  if ( mCellHeight < 4 * mAgenda->gridSpacingY() ) {
       mCellHeight = 4 * mAgenda->gridSpacingY();
  }

  updateGeometry();

  repaint();
}

/**  */
void TimeLabels::setAgenda( Agenda *agenda )
{
  mAgenda = agenda;

  if ( mAgenda ) {
    connect( mAgenda, SIGNAL(mousePosSignal(QPoint)),
             SLOT(mousePosChanged(QPoint)) );
    connect( mAgenda, SIGNAL(enterAgenda()), SLOT(showMousePos()) );
    connect( mAgenda, SIGNAL(leaveAgenda()), SLOT(hideMousePos()) );
    connect( mAgenda, SIGNAL(gridSpacingYChanged(double)),
             SLOT(setCellHeight(double)) );
  }
}

/** This is called in response to repaint() */
void TimeLabels::paintEvent( QPaintEvent * )
{
  QPainter p( this );

  const int ch = height();

  // We won't paint parts that aren't visible
  const int cy = -y();// y() returns a negative value.

  const int beginning =
    !mSpec.isValid() ?
    0 :
    ( mSpec.timeZone().currentOffset() -
      mTimeLabelsZone->preferences()->timeSpec().timeZone().currentOffset() ) / ( 60 * 60 );

  // bug:  the parameters cx and cw are the areas that need to be
  //       redrawn, not the area of the widget.  unfortunately, this
  //       code assumes the latter...

  // now, for a workaround...
  const int cx = frameWidth() * 2;
  const int cw = width();
  // end of workaround

  int cell = static_cast<int>( cy / mCellHeight ) + beginning;  // the hour we start drawing with
  double y = ( cell - beginning ) * mCellHeight;
  QFontMetrics fm = fontMetrics();
  QString hour;
  int timeHeight = fm.ascent();
  QFont hourFont = mTimeLabelsZone->preferences()->agendaTimeLabelsFont();
  p.setFont( font() );

  //TODO: rewrite this using KLocale's time formats. "am/pm" doesn't make sense
  // in some locale's
  QString suffix;
  if ( !KLocale::global()->use12Clock() ) {
    suffix = QLatin1String("00");
  } else {
    suffix = QLatin1String("am");
    if ( cell > 11 ) {
      suffix = QLatin1String("pm");
    }
  }

  // We adjust the size of the hour font to keep it reasonable
  if ( timeHeight > mCellHeight ) {
    timeHeight = static_cast<int>( mCellHeight - 1 );
    int pointS = hourFont.pointSize();
    while ( pointS > 4 ) { // TODO: use smallestReadableFont() when added to kdelibs
      hourFont.setPointSize( pointS );
      fm = QFontMetrics( hourFont );
      if ( fm.ascent() < mCellHeight ) {
        break;
      }
      --pointS;
    }
    fm = QFontMetrics( hourFont );
    timeHeight = fm.ascent();
  }
  //timeHeight -= (timeHeight/4-2);
  QFont suffixFont = hourFont;
  suffixFont.setPointSize( suffixFont.pointSize() / 2 );
  QFontMetrics fmS( suffixFont );
  const int startW = cw - frameWidth() - 2 ;
  const int tw2 = fmS.width( suffix );
  const int divTimeHeight = ( timeHeight - 1 ) / 2 - 1;
  //testline
  //p->drawLine(0,0,0,contentsHeight());
  while ( y < cy + ch + mCellHeight ) {
    QColor lineColor, textColor;
    textColor = palette().color( QPalette::WindowText );
    if ( cell < 0 || cell >= 24 ) {
      textColor.setAlphaF( 0.5 );
    }
    lineColor = textColor;
    lineColor.setAlphaF( lineColor.alphaF() / 5. );
    p.setPen( lineColor );

    // hour, full line
    p.drawLine( cx, int( y ), cw + 2, int( y ) );

    hour.setNum( cell % 24 );
    // handle different timezones
    if ( cell < 0 ) {
      hour.setNum( cell + 24 );
    }
    // handle 24h and am/pm time formats
    if ( KLocale::global()->use12Clock() ) {
      if ( cell == 12 ) {
        suffix =QLatin1String( "pm");
      }
      if ( cell == 0 ) {
        hour.setNum( 12 );
      }
      if ( cell > 12 ) {
        hour.setNum( cell - 12 );
      }
    }

    // draw the time label
    p.setPen( textColor );
    const int timeWidth = fm.width( hour );
    int offset = startW - timeWidth - tw2 -1 ;
    p.setFont( hourFont );
    p.drawText( offset, static_cast<int>( y + timeHeight ), hour );
    p.setFont( suffixFont );
    offset = startW - tw2;
    p.drawText( offset, static_cast<int>( y + timeHeight - divTimeHeight ), suffix );

    // increment indices
    y += mCellHeight;
    cell++;
  }
}

QSize TimeLabels::sizeHint() const
{
  return QSize( mMiniWidth, mRows * mCellHeight );
}

void TimeLabels::contextMenuEvent( QContextMenuEvent *event )
{
  Q_UNUSED( event );

  QMenu popup( this );
  QAction *editTimeZones =
    popup.addAction( KIcon( QLatin1String("document-properties") ), i18n( "&Add Timezones..." ) );
  QAction *removeTimeZone =
    popup.addAction( KIcon( QLatin1String("edit-delete") ),
                     i18n( "&Remove Timezone %1", i18n( mSpec.timeZone().name().toUtf8() ) ) );
  if ( !mSpec.isValid() ||
       !mTimeLabelsZone->preferences()->timeScaleTimezones().count() ||
       mSpec == mTimeLabelsZone->preferences()->timeSpec() ) {
    removeTimeZone->setEnabled( false );
  }

  QAction *activatedAction = popup.exec( QCursor::pos() );
  if ( activatedAction == editTimeZones ) {
    QPointer<TimeScaleConfigDialog> dialog =
      new TimeScaleConfigDialog( mTimeLabelsZone->preferences(), this );
    if ( dialog->exec() == QDialog::Accepted ) {
      mTimeLabelsZone->reset();
    }
    delete dialog;
  } else if ( activatedAction == removeTimeZone ) {
    QStringList list = mTimeLabelsZone->preferences()->timeScaleTimezones();
    list.removeAll( mSpec.timeZone().name() );
    mTimeLabelsZone->preferences()->setTimeScaleTimezones( list );
    mTimeLabelsZone->preferences()->writeConfig();
    mTimeLabelsZone->reset();
    hide();
    deleteLater();
  }
}

KDateTime::Spec TimeLabels::timeSpec()
{
  return mSpec;
}

QString TimeLabels::header() const
{
  return i18n( mSpec.timeZone().name().toUtf8() );
}

QString TimeLabels::headerToolTip() const
{
  KTimeZone tz = mSpec.timeZone();

  QString toolTip;
  toolTip += QLatin1String("<qt>");
  toolTip += i18n( "<b>%1</b>", i18n( tz.name().toUtf8() ) );
  toolTip += QLatin1String("<hr>");
  //TODO: Once string freeze is lifted, add UTC offset here
  if ( !tz.countryCode().isEmpty() ) {
    toolTip += i18n( "<i>Country Code:</i> %1", tz.countryCode() );
    toolTip += QLatin1String("<br/>");
  }
  if ( !tz.abbreviations().isEmpty() ) {
    toolTip += i18n( "<i>Abbreviations:</i>" ) + QLatin1String("</i>");
    toolTip += QLatin1String("&nbsp;");
    foreach ( const QByteArray &a, tz.abbreviations() ) {
      toolTip += QString::fromLocal8Bit( a );
      toolTip += QLatin1String(",&nbsp;");
    }
    toolTip.chop( 7 );
    toolTip += QLatin1String("<br/>");
  }
  if ( !tz.comment().isEmpty() ) {
    toolTip += i18n( "<i>Comment:</i> %1", tz.comment() );
  }
  toolTip += QLatin1String("</qt>");

  return toolTip;
}

