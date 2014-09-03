/* Copyright (C) 2011, 2012, 2013, 2014 Laurent Montel <montel@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "sieveeditor.h"
#include "sieve-editor.h"
#include "sieveeditorwidget.h"

#include <KLocalizedString>

#include <QPushButton>
#include <QKeyEvent>
#include <KSharedConfig>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QVBoxLayout>

using namespace KSieveUi;

SieveEditor::SieveEditor(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18n("Edit Sieve Script"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mOkButton = buttonBox->button(QDialogButtonBox::Ok);
    mOkButton->setDefault(true);
    mOkButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &SieveEditor::slotAccepted);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &SieveEditor::slotCanceled);
    setModal(true);
    mSieveEditorWidget = new SieveEditorWidget;
    connect(mSieveEditorWidget, SIGNAL(valueChanged(bool)), this, SIGNAL(valueChanged(bool)));
    mainLayout->addWidget(mSieveEditorWidget);
    mainLayout->addWidget(buttonBox);
    connect(mSieveEditorWidget, &SieveEditorWidget::enableButtonOk, this, &SieveEditor::slotEnableButtonOk);
    connect(mSieveEditorWidget, SIGNAL(checkSyntax()), this, SIGNAL(checkSyntax()));
    readConfig();
}

SieveEditor::~SieveEditor()
{
    writeConfig();
}

void SieveEditor::slotAccepted()
{
   Q_EMIT okClicked();
   accept();
}

void SieveEditor::slotCanceled()
{
   Q_EMIT cancelClicked();
   reject();
}


bool SieveEditor::event(QEvent *e)
{
    // Close the bar when pressing Escape.
    // Not using a QShortcut for this because it could conflict with
    // window-global actions (e.g. Emil Sedgh binds Esc to "close tab").
    // With a shortcut override we can catch this before it gets to kactions.
    const bool shortCutOverride = (e->type() == QEvent::ShortcutOverride);
    if (shortCutOverride || e->type() == QEvent::KeyPress) {
        QKeyEvent *kev = static_cast<QKeyEvent * >(e);
        if (kev->key() == Qt::Key_Escape) {
            e->ignore();
            return true;
        }
    }
    return QDialog::event(e);
}

void SieveEditor::slotEnableButtonOk(bool b)
{
    mOkButton->setEnabled(b);
}

void SieveEditor::writeConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "SieveEditor");
    group.writeEntry("Size", size());
}

void SieveEditor::readConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "SieveEditor");
    const QSize sizeDialog = group.readEntry("Size", QSize(800, 600));
    if (sizeDialog.isValid()) {
        resize(sizeDialog);
    }
}

QString SieveEditor::script() const
{
    return mSieveEditorWidget->script();
}

QString SieveEditor::originalScript() const
{
    return mSieveEditorWidget->originalScript();
}

void SieveEditor::setScript(const QString &script)
{
    mSieveEditorWidget->setScript(script);
}

void SieveEditor::setDebugScript(const QString &debug)
{
    mSieveEditorWidget->setDebugScript(debug);
}

void SieveEditor::setScriptName(const QString &name)
{
    mSieveEditorWidget->setScriptName(name);
}

void SieveEditor::resultDone()
{
    mSieveEditorWidget->resultDone();
}

void SieveEditor::setSieveCapabilities(const QStringList &capabilities)
{
    mSieveEditorWidget->setSieveCapabilities(capabilities);
}

void SieveEditor::addFailedMessage(const QString &err)
{
    mSieveEditorWidget->addFailedMessage(err);
}

void SieveEditor::addOkMessage(const QString &msg)
{
    mSieveEditorWidget->addOkMessage(msg);
}

