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

#include "scamdetectionwarningwidget.h"
#include "globalsettings.h"

#include <KLocale>
#include <KAction>

using namespace MessageViewer;

ScamDetectionWarningWidget::ScamDetectionWarningWidget(QWidget *parent)
    : KMessageWidget(parent)
{
    setVisible(false);
    setCloseButtonVisible(true);
    setMessageType(Warning);
    setWordWrap(true);
    setText(i18n("This message may be a scam."));


    KAction *action = new KAction( i18n( "Details" ), this );
    connect( action, SIGNAL(triggered(bool)), SIGNAL(showDetails()) );
    addAction( action );

    action = new KAction( i18n( "Move to Trash" ), this );
    connect( action, SIGNAL(triggered(bool)), SIGNAL(moveMessageToTrash()) );
    addAction( action );

    action = new KAction( i18n( "Disable scam detection for all messages" ), this );
    connect( action, SIGNAL(triggered(bool)), SLOT(slotDisableScamDetection()) );
    addAction( action );
}

ScamDetectionWarningWidget::~ScamDetectionWarningWidget()
{
}

void ScamDetectionWarningWidget::slotShowWarning()
{
    setVisible(true);
}

void ScamDetectionWarningWidget::slotDisableScamDetection()
{
    MessageViewer::GlobalSettings::self()->setScamDetectionEnabled( false );
    MessageViewer::GlobalSettings::self()->writeConfig();
    setVisible(false);
}

#include "scamdetectionwarningwidget.moc"
