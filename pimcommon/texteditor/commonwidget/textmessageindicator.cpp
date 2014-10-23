/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>
  based on code from okular PageViewMessage

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

#include "textmessageindicator.h"

#include <KIconLoader>

#include <QAbstractScrollArea>
#include <QPainter>
#include <QTimer>
#include <QResizeEvent>
#include <QApplication>
#include <QDebug>

using namespace PimCommon;
TextMessageIndicator::TextMessageIndicator( QWidget * parent )
    : QWidget( parent ),
      mTimer( 0 ),
      mLineSpacing( 0 )
{
    setObjectName( QLatin1String( "TextMessageIndicator" ) );
    setFocusPolicy( Qt::NoFocus );
    QPalette pal = palette();
    pal.setColor( QPalette::Active, QPalette::Window, QApplication::palette().highlight().color());
    setPalette( pal );
    // if the layout is LtR, we can safely place it in the right position
    if ( layoutDirection() == Qt::LeftToRight )
        move( 10, parentWidget()->height() - 10 );
    resize( 0, 0 );
    hide();
}

void TextMessageIndicator::display( const QString & message, const QString & details, Icon icon, int durationMs )
{
    if (message.isEmpty()) {
        return;
    }
    // set text
    mMessage = message;
    mDetails = details;
    // reset vars
    mLineSpacing = 0;
    // load icon (if set)
    mSymbol = QPixmap();
    if ( icon != None ) {
        switch ( icon )
        {
        case Error:
            mSymbol = SmallIcon( QLatin1String("dialog-error") );
            break;
        case Warning:
            mSymbol = SmallIcon( QLatin1String("dialog-warning") );
            break;
        default:
            mSymbol = SmallIcon( QLatin1String("dialog-information") );
            break;
        }
    }

    computeSizeAndResize();
    // show widget and schedule a repaint
    show();
    update();

    // close the message window after given mS
    if ( durationMs > 0 ) {
        if ( !mTimer ) {
            mTimer = new QTimer( this );
            mTimer->setSingleShot( true );
            connect(mTimer, &QTimer::timeout, this, &TextMessageIndicator::hide);
        }
        mTimer->start( durationMs );
    } else if ( mTimer )
        mTimer->stop();

    qobject_cast<QAbstractScrollArea*>(parentWidget())->viewport()->installEventFilter(this);

}

QRect TextMessageIndicator::computeTextRect( const QString & message, int extra_width ) const
// Return the QRect which embeds the text
{
    int charSize = fontMetrics().averageCharWidth();
    /* width of the viewport, minus 20 (~ size removed by further resizing),
       minus the extra size (usually the icon width), minus (a bit empirical)
       twice the mean width of a character to ensure that the bounding box is
       really smaller than the container.
     */
    const int boundingWidth = qobject_cast<QAbstractScrollArea*>(parentWidget())->viewport()->width() - 20 - ( extra_width > 0 ? 2 + extra_width : 0 ) - 2*charSize;
    QRect textRect = fontMetrics().boundingRect( 0, 0, boundingWidth, 0,
                                                 Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, message );
    textRect.translate( -textRect.left(), -textRect.top() );
    textRect.adjust( 0, 0, 2, 2 );

    return textRect;
}

void TextMessageIndicator::computeSizeAndResize()
{
    // determine text rectangle
    const QRect textRect = computeTextRect( mMessage, mSymbol.width() );
    int width = textRect.width(),
            height = textRect.height();

    if ( !mDetails.isEmpty() ) {
        // determine details text rectangle
        const QRect detailsRect = computeTextRect( mDetails, mSymbol.width() );
        width = qMax( width, detailsRect.width() );
        height += detailsRect.height();

        // plus add a ~60% line spacing
        mLineSpacing = static_cast< int >( fontMetrics().height() * 0.6 );
        height += mLineSpacing;
    }

    // update geometry with icon information
    if ( !mSymbol.isNull() ) {
        width += 2 + mSymbol.width();
        height = qMax( height, mSymbol.height() );
    }

    // resize widget
    resize( QRect( 0, 0, width + 10, height + 8 ).size() );

    // if the layout is RtL, we can move it to the right place only after we
    // know how much size it will take
    int posX = parentWidget()->width() - geometry().width() - 20 - 1;
    if ( layoutDirection() == Qt::RightToLeft )
        posX = 10;
    move( posX, parentWidget()->height() - geometry().height() - 20 );
}


bool TextMessageIndicator::eventFilter(QObject * obj, QEvent * event )
{
    /* if the parent object (scroll area) resizes, the message should
       resize as well */
    if (event->type() == QEvent::Resize) {
        QResizeEvent *resizeEvent = static_cast<QResizeEvent *>(event);
        if ( resizeEvent->oldSize() != resizeEvent->size() ) {
            computeSizeAndResize();
        }
    }
    // standard event processing
    return QObject::eventFilter(obj, event);
}

void TextMessageIndicator::paintEvent( QPaintEvent * /* e */ )
{
    const QRect textRect = computeTextRect( mMessage, mSymbol.width() );

    QRect detailsRect;
    if ( !mDetails.isEmpty() ) {
        detailsRect = computeTextRect( mDetails, mSymbol.width() );
    }

    int textXOffset = 0;
    // add 2 to account for the reduced drawRoundRect later
    int textYOffset = ( geometry().height() - textRect.height() - detailsRect.height() - mLineSpacing + 2 ) / 2;
    int iconXOffset = 0;
    int iconYOffset = !mSymbol.isNull() ? ( geometry().height() - mSymbol.height() ) / 2 : 0;
    int shadowOffset = 1;

    if ( layoutDirection() == Qt::RightToLeft )
        iconXOffset = 2 + textRect.width();
    else
        textXOffset = 2 + mSymbol.width();

    // draw background
    QPainter painter( this );
    painter.setRenderHint( QPainter::Antialiasing, true );
    painter.setPen( Qt::black );
    painter.setBrush( palette().color( QPalette::Window ) );
    painter.translate( 0.5, 0.5 );
    painter.drawRoundRect( 1, 1, width()-2, height()-2, 1600 / width(), 1600 / height() );

    // draw icon if present
    if ( !mSymbol.isNull() )
        painter.drawPixmap( 5 + iconXOffset, iconYOffset, mSymbol, 0, 0, mSymbol.width(), mSymbol.height() );

    const int xStartPoint = 5 + textXOffset;
    const int yStartPoint = textYOffset;
    const int textDrawingFlags = Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap;

    // draw shadow and text
    painter.setPen( palette().color( QPalette::Window ).dark( 115 ) );
    painter.drawText( xStartPoint + shadowOffset, yStartPoint + shadowOffset, textRect.width(), textRect.height(), textDrawingFlags, mMessage );
    if ( !mDetails.isEmpty() )
        painter.drawText( xStartPoint + shadowOffset, yStartPoint + textRect.height() + mLineSpacing + shadowOffset, textRect.width(), detailsRect.height(), textDrawingFlags, mDetails );
    painter.setPen( palette().color( QPalette::WindowText ) );
    painter.drawText( xStartPoint, yStartPoint, textRect.width(), textRect.height(), textDrawingFlags, mMessage );
    if ( !mDetails.isEmpty() )
        painter.drawText( xStartPoint + shadowOffset, yStartPoint + textRect.height() + mLineSpacing, textRect.width(), detailsRect.height(), textDrawingFlags, mDetails );
}

void TextMessageIndicator::mousePressEvent( QMouseEvent * /*e*/ )
{
    if ( mTimer )
        mTimer->stop();
    hide();
}

