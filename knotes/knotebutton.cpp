/*******************************************************************
 KNotes -- Notes for the KDE project

 Copyright (c) 2002-2004, The KNotes Developers

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*******************************************************************/

#include <tqstyle.h>
#include <tqpainter.h>
#include <tqiconset.h>
#include <tqsizepolicy.h>

#include <kglobal.h>
#include <kicontheme.h>
#include <kiconloader.h>

#include "knotebutton.h"


KNoteButton::KNoteButton( const TQString& icon, TQWidget *parent, const char *name )
    : TQPushButton( parent, name )
{
    setFocusPolicy( NoFocus );
    setSizePolicy( TQSizePolicy( TQSizePolicy::Fixed, TQSizePolicy::Fixed ) );

    m_flat = true;

    if ( !icon.isEmpty() )
        setIconSet( KGlobal::iconLoader()->loadIconSet( icon, KIcon::Small, 10 ) );
}

KNoteButton::~KNoteButton()
{
}

void KNoteButton::enterEvent( TQEvent * )
{
    m_flat = false;
    repaint( false );
}

void KNoteButton::leaveEvent( TQEvent * )
{
    m_flat = true;
    repaint();
}

TQSize KNoteButton::sizeHint() const
{
    return TQSize( TQPushButton::sizeHint().height(), TQPushButton::sizeHint().height() );
}

void KNoteButton::drawButton( TQPainter* p )
{
    TQStyle::SFlags flags = TQStyle::Style_Default;

    if ( isEnabled() )
        flags |= TQStyle::Style_Enabled;
    if ( isDown() )
        flags |= TQStyle::Style_Down;
    if ( isOn() )
        flags |= TQStyle::Style_On;
    if ( !isFlat() && !isDown() )
        flags |= TQStyle::Style_Raised;
    if ( !m_flat )
        flags |= TQStyle::Style_MouseOver;

    style().drawPrimitive( TQStyle::PE_ButtonTool, p, rect(), colorGroup(), flags );
    drawButtonLabel( p );
}

void KNoteButton::drawButtonLabel( TQPainter* p )
{
    if ( iconSet() && !iconSet()->isNull() )
    {
        TQIconSet::Mode  mode  = TQIconSet::Disabled;
        TQIconSet::State state = TQIconSet::Off;

        if ( isEnabled() )
            mode = hasFocus() ? TQIconSet::Active : TQIconSet::Normal;
        if ( isToggleButton() && isOn() )
            state = TQIconSet::On;

        TQPixmap pix = iconSet()->pixmap( TQIconSet::Small, mode, state );

        int dx = ( width() - pix.width() ) / 2;
        int dy = ( height() - pix.height() ) / 2;

        // Shift button contents if pushed.
        if ( isOn() || isDown() )
        {
            dx += style().pixelMetric( TQStyle::PM_ButtonShiftHorizontal, this );
            dy += style().pixelMetric( TQStyle::PM_ButtonShiftVertical, this );
        }

        p->drawPixmap( dx, dy, pix );
    }
}
