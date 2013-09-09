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

#include "sieveeditorparsingwarning.h"
#include "autocreatescripts/sievescriptparsingerrordialog.h"

#include <KLocale>
#include <KAction>

#include <QPointer>

using namespace KSieveUi;

SieveEditorParsingWarning::SieveEditorParsingWarning(QWidget *parent)
    : KMessageWidget(parent)
{
    setVisible(false);
    setCloseButtonVisible(false);
    setMessageType(Error);
    setText(i18n("Some errors were found during parsing. <a href=\"sieveerrordetails\">(Details...)</a>"));
    connect(this, SIGNAL(linkActivated(QString)), SLOT(slotShowDetails(QString)));

    KAction *action = new KAction( i18n( "Switch in graphical mode" ), this );
    connect( action, SIGNAL(triggered(bool)), SLOT(slotSwitchInGraphicalMode()) );
    addAction( action );

    action = new KAction( i18n( "Keep in text mode" ), this );
    connect( action, SIGNAL(triggered(bool)), SLOT(slotKeepInTextMode()) );
    addAction( action );

    setWordWrap(true);
}

SieveEditorParsingWarning::~SieveEditorParsingWarning()
{
}

void SieveEditorParsingWarning::slotShowDetails(const QString &content)
{
    if (content == QLatin1String("sieveerrordetails")) {
        QPointer<SieveScriptParsingErrorDialog> dlg = new SieveScriptParsingErrorDialog(this);
        dlg->setError(mScript, mErrors);
        dlg->exec();
        delete dlg;
    }
}

void SieveEditorParsingWarning::setErrors(const QString &errors, const QString &initialScript)
{
    mErrors = errors;
    mScript = initialScript;
}

void SieveEditorParsingWarning::slotSwitchInGraphicalMode()
{
    Q_EMIT switchToGraphicalMode();
    setVisible(false);
}

void SieveEditorParsingWarning::slotKeepInTextMode()
{
    setVisible(false);
}

#include "sieveeditorparsingwarning.moc"
