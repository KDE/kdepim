/* Copyright (C) 2011-2015 Laurent Montel <montel@kde.org>
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
class KSieveUi::SieveEditorPrivate
{
public:
    SieveEditorPrivate()
        : mSieveEditorWidget(Q_NULLPTR),
          mOkButton(Q_NULLPTR)
    {

    }
    SieveEditorWidget *mSieveEditorWidget;
    QPushButton *mOkButton;
};

SieveEditor::SieveEditor(QWidget *parent)
    : QDialog(parent),
      d(new KSieveUi::SieveEditorPrivate)
{
    setWindowTitle(i18n("Edit Sieve Script"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    d->mOkButton = buttonBox->button(QDialogButtonBox::Ok);
    d->mOkButton->setDefault(true);
    d->mOkButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &SieveEditor::slotAccepted);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &SieveEditor::slotCanceled);
    d->mSieveEditorWidget = new SieveEditorWidget(true);
    connect(d->mSieveEditorWidget, &SieveEditorWidget::valueChanged, this, &SieveEditor::valueChanged);
    mainLayout->addWidget(d->mSieveEditorWidget);
    mainLayout->addWidget(buttonBox);
    connect(d->mSieveEditorWidget, &SieveEditorWidget::enableButtonOk, this, &SieveEditor::slotEnableButtonOk);
    connect(this, &SieveEditor::finished, this, &SieveEditor::cancelClicked);
    connect(d->mSieveEditorWidget, &SieveEditorWidget::checkSyntax, this, &SieveEditor::checkSyntax);
    readConfig();
}

SieveEditor::~SieveEditor()
{
    writeConfig();
    delete d;
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
    // With a shortcut Q_DECL_OVERRIDE we can catch this before it gets to kactions.
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
    d->mOkButton->setEnabled(b);
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
    return d->mSieveEditorWidget->script();
}

QString SieveEditor::originalScript() const
{
    return d->mSieveEditorWidget->originalScript();
}

void SieveEditor::setScript(const QString &script)
{
    d->mSieveEditorWidget->setScript(script);
}

void SieveEditor::setDebugScript(const QString &debug)
{
    d->mSieveEditorWidget->setDebugScript(debug);
}

void SieveEditor::setScriptName(const QString &name)
{
    d->mSieveEditorWidget->setScriptName(name);
}

void SieveEditor::resultDone()
{
    d->mSieveEditorWidget->resultDone();
}

void SieveEditor::setSieveCapabilities(const QStringList &capabilities)
{
    d->mSieveEditorWidget->setSieveCapabilities(capabilities);
}

void SieveEditor::addFailedMessage(const QString &err)
{
    d->mSieveEditorWidget->addFailedMessage(err);
}

void SieveEditor::addOkMessage(const QString &msg)
{
    d->mSieveEditorWidget->addOkMessage(msg);
}

