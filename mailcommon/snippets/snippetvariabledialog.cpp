/*
  Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net,
    Author: Tobias Koenig <tokoe@kdab.com>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

#include "snippetvariabledialog_p.h"

#include <KLocalizedString>
#include <KTextEdit>

#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>

using namespace MailCommon;

SnippetVariableDialog::SnippetVariableDialog(const QString &variableName,
        QMap<QString, QString> *variables,
        QWidget *parent)
    : QDialog(parent), mVariableName(variableName), mVariables(variables)
{
    setWindowTitle(i18n("Enter Values for Variables"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &SnippetVariableDialog::slotAccepted);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &SnippetVariableDialog::reject);
    mainLayout->addWidget(buttonBox);

    QVBoxLayout *layout = new QVBoxLayout(mainWidget);

    QLabel *label = new QLabel;
    label->setText(i18n("Enter the replacement values for '%1':", variableName));
    layout->addWidget(label);

    mVariableValueText = new KTextEdit;
    mVariableValueText->setAcceptRichText(false);
    layout->addWidget(mVariableValueText);

    mSaveVariable = new QCheckBox;
    mSaveVariable->setChecked(false);
    mSaveVariable->setText(i18n("Make value &default"));
    mSaveVariable->setToolTip(
        i18nc("@info:tooltip",
              "Enable this to save the value entered to the right "
              "as the default value for this variable"));
    mSaveVariable->setWhatsThis(
        i18nc("@info:whatsthis",
              "If you enable this option, the value entered to the right will be saved. "
              "If you use the same variable later, even in another snippet, the value entered "
              "to the right will be the default value for that variable."));
    layout->addWidget(mSaveVariable);

    if (mVariables->contains(variableName)) {
        mSaveVariable->setChecked(true);
        mVariableValueText->setText(mVariables->value(variableName));
    }

    mVariableValueText->setFocus();
}

QString SnippetVariableDialog::variableValue() const
{
    return mVariableValueText->toPlainText();
}

bool SnippetVariableDialog::saveVariableIsChecked() const
{
    return mSaveVariable->isChecked();
}

void SnippetVariableDialog::slotAccepted()
{
    if (mSaveVariable->isChecked()) {
        mVariables->insert(mVariableName, mVariableValueText->toPlainText());
    } else {
        mVariables->remove(mVariableName);
    }

    accept();
}

#include "moc_snippetvariabledialog_p.cpp"
