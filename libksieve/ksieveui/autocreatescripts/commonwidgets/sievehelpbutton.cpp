/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include "sievehelpbutton.h"

#include <KLocalizedString>
#include <KIcon>

#include <QWhatsThisClickedEvent>
#include <QDebug>

using namespace KSieveUi;
SieveHelpButton::SieveHelpButton(QWidget *parent)
    : QToolButton(parent)
{
    setToolTip(i18n("Help"));
    setIcon( KIcon( QLatin1String("help-hint") ) );
}

SieveHelpButton::~SieveHelpButton()
{

}


bool SieveHelpButton::event(QEvent* event)
{
    if (event->type() == QEvent::WhatsThisClicked)
    {
        QWhatsThisClickedEvent* clicked = static_cast<QWhatsThisClickedEvent*>(event);
        qDebug()<<" clicked->href() "<<clicked->href();
        return true;
    }
    return QToolButton::event(event);
}
