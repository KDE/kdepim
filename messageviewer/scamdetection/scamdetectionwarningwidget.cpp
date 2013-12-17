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
#include "settings/globalsettings.h"

#include <KLocalizedString>
#include <KAction>

#include <QDebug>

using namespace MessageViewer;

ScamDetectionWarningWidget::ScamDetectionWarningWidget(QWidget *parent)
    : KMessageWidget(parent),
      mUseInTestApps(false)
{
    setVisible(false);
    setCloseButtonVisible(true);
    setMessageType(Warning);
    setWordWrap(true);
    setText(i18n("This message may be a scam. <a href=\"scamdetails\">(Details...)</a>"));

    connect(this, SIGNAL(linkActivated(QString)), SLOT(slotShowDetails(QString)));

    KAction *action = new KAction( i18n( "Move to Trash" ), this );
    connect( action, SIGNAL(triggered(bool)), SIGNAL(moveMessageToTrash()) );
    addAction( action );

    action = new KAction( i18n( "I confirm it's not a scam" ), this );
    connect( action, SIGNAL(triggered(bool)), SLOT(slotMessageIsNotAScam()) );
    addAction( action );

    action = new KAction( i18n( "Add email to whitelist" ), this );
    connect( action, SIGNAL(triggered(bool)), SLOT(slotAddToWhiteList()) );
    addAction( action );

    action = new KAction( i18n( "Disable scam detection for all messages" ), this );
    connect( action, SIGNAL(triggered(bool)), SLOT(slotDisableScamDetection()) );
    addAction( action );
}

ScamDetectionWarningWidget::~ScamDetectionWarningWidget()
{
}

void ScamDetectionWarningWidget::setUseInTestApps(bool b)
{
    mUseInTestApps = b;
}

void ScamDetectionWarningWidget::slotMessageIsNotAScam()
{
    Q_EMIT messageIsNotAScam();
    setVisible(false);
}

void ScamDetectionWarningWidget::slotShowDetails(const QString &content)
{
    if (content == QLatin1String("scamdetails")) {
        Q_EMIT showDetails();
    }
}

void ScamDetectionWarningWidget::slotShowWarning()
{
    animatedShow();
}

void ScamDetectionWarningWidget::slotDisableScamDetection()
{
    if (!mUseInTestApps) {
        MessageViewer::GlobalSettings::self()->setScamDetectionEnabled( false );
        MessageViewer::GlobalSettings::self()->writeConfig();
    }
    setVisible(false);
}

void ScamDetectionWarningWidget::slotAddToWhiteList()
{
    setVisible(false);
    Q_EMIT addToWhiteList();
}

