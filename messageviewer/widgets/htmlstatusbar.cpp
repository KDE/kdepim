/*  -*- c++ -*-
    htmlstatusbar.cpp

    This file is part of KMail, the KDE mail client.
    Copyright (c) 2002 Ingo Kloecker <kloecker@kde.org>
    Copyright (c) 2003 Marc Mutz <mutz@kde.org>
    Copyright (c) 2013 Laurent Montel <montel@kde.org>

    KMail is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License, version 2, as
    published by the Free Software Foundation.

    KMail is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include "htmlstatusbar.h"
#include "settings/globalsettings.h"

#include "messagecore/settings/globalsettings.h"

#include <KLocalizedString>
#include <kconfiggroup.h>

#include <QMouseEvent>

using namespace MessageViewer;

HtmlStatusBar::HtmlStatusBar(QWidget * parent)
    : QLabel( parent ),
      mMode( Util::Normal )
{
    setAlignment( Qt::AlignHCenter | Qt::AlignTop );
    setAutoFillBackground( true );
    update();
}

HtmlStatusBar::~HtmlStatusBar() {}

void HtmlStatusBar::update()
{
    QPalette pal = palette();
    pal.setColor( backgroundRole(), bgColor() );
    pal.setColor( foregroundRole(), fgColor() );
    setPalette( pal );
    setText( message() );
    setToolTip( toolTip() );
}

void HtmlStatusBar::setNormalMode()
{
    setMode( Util::Normal );
}

void HtmlStatusBar::setHtmlMode()
{
    setMode( Util::Html );
}

void HtmlStatusBar::setMultipartPlainMode()
{
    setMode( Util::MultipartPlain );
}

void HtmlStatusBar::setMultipartHtmlMode()
{
    setMode( Util::MultipartHtml );
}

void HtmlStatusBar::setMode( Util::HtmlMode m, UpdateMode mode )
{
    mMode = m;
    if ( mode == Update )
        update();
}

void HtmlStatusBar::mousePressEvent( QMouseEvent * event )
{
    if ( event->button() == Qt::LeftButton ) {
        emit clicked();
    }
}

QString HtmlStatusBar::message() const {
    switch ( mode() ) {
    case Util::Html: // bold: "HTML Message"
    case Util::MultipartHtml:
        return i18nc( "'HTML Message' with html linebreaks between each letter and in bold text.",
                      "<qt><b><br />H<br />T<br />M<br />L<br /> "
                      "<br />M<br />e<br />s<br />s<br />a<br />g<br />e</b></qt>" );
    case Util::Normal: // normal: "No HTML Message"
        return i18nc("'No HTML Message' with html linebreaks between each letter.",
                     "<qt><br />N<br />o<br /> "
                     "<br />H<br />T<br />M<br />L<br /> "
                     "<br />M<br />e<br />s<br />s<br />a<br />g<br />e</qt>" );
    case Util::MultipartPlain: // normal: "Plain Message"
        return i18nc("'Plain Message' with html linebreaks between each letter.",
                     "<qt><br />P<br />l<br />a<br />i<br />n<br /> "
                     "<br />M<br />e<br />s<br />s<br />a<br />g<br />e<br /></qt>" );
    default:
        return QString();
    }
}

QString HtmlStatusBar::toolTip() const
{
    switch ( mode() )
    {
    case Util::Html:
    case Util::MultipartHtml:
    case Util::MultipartPlain:
        return i18n( "Click to toggle between HTML and plain text." );
    default:
    case Util::Normal:
        break;
    }

    return QString();
}

QColor HtmlStatusBar::fgColor() const
{
    KConfigGroup conf( GlobalSettings::self()->config(), "Reader" );
    QColor defaultColor, color;
    switch ( mode() ) {
    case Util::Html:
    case Util::MultipartHtml:
        defaultColor = Qt::white;
        color = defaultColor;
        if ( !MessageCore::GlobalSettings::self()->useDefaultColors() ) {
            color = conf.readEntry( "ColorbarForegroundHTML", defaultColor );
        }
        return color;
    case Util::Normal:
    case Util::MultipartPlain:
        defaultColor = Qt::black;
        color = defaultColor;
        if ( !MessageCore::GlobalSettings::self()->useDefaultColors() ) {
            color = conf.readEntry( "ColorbarForegroundPlain", defaultColor );
        }
        return color;
    default:
        return Qt::black;
    }
}

QColor HtmlStatusBar::bgColor() const {
    KConfigGroup conf( GlobalSettings::self()->config(), "Reader" );

    QColor defaultColor, color;
    switch ( mode() ) {
    case Util::Html:
    case Util::MultipartHtml:
        defaultColor = Qt::black;
        color = defaultColor;
        if ( !MessageCore::GlobalSettings::self()->useDefaultColors() ) {
            color = conf.readEntry( "ColorbarBackgroundHTML", defaultColor );
        }
        return color;
    case Util::Normal:
    case Util::MultipartPlain:
        defaultColor = Qt::lightGray;
        color = defaultColor;
        if ( !MessageCore::GlobalSettings::self()->useDefaultColors() ) {
            color = conf.readEntry( "ColorbarBackgroundPlain", defaultColor );
        }
        return color;
    default:
        return Qt::white;
    }
}

