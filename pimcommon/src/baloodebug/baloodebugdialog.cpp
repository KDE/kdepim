/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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

#include "baloodebugdialog.h"
#include "baloodebugwidget.h"
#include "util/pimutil.h"

#include <KLocalizedString>
#include <QVBoxLayout>
#include <KSharedConfig>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>

using namespace PimCommon;

class PimCommon::BalooDebugDialogPrivate
{
public:
    BalooDebugDialogPrivate()
        : mBalooDebugWidget(Q_NULLPTR)
    {

    }

    BalooDebugWidget *mBalooDebugWidget;
};

BalooDebugDialog::BalooDebugDialog(QWidget *parent)
    : QDialog(parent),
      d(new PimCommon::BalooDebugDialogPrivate)
{
    //Don't translate it's just a dialog to debug
    setWindowTitle(QStringLiteral("Debug baloo"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    QPushButton *user1Button = new QPushButton;
    buttonBox->addButton(user1Button, QDialogButtonBox::ActionRole);
    //Don't translate it.
    user1Button->setText(QStringLiteral("Save As..."));
    connect(user1Button, &QPushButton::clicked, this, &BalooDebugDialog::slotSaveAs);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &BalooDebugDialog::reject);
    d->mBalooDebugWidget = new BalooDebugWidget(this);
    d->mBalooDebugWidget->setObjectName(QStringLiteral("baloodebugwidget"));
    mainLayout->addWidget(d->mBalooDebugWidget);
    mainLayout->addWidget(buttonBox);
    readConfig();
}

BalooDebugDialog::~BalooDebugDialog()
{
    writeConfig();
    delete d;
}

void BalooDebugDialog::readConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "BalooDebugDialog");
    const QSize sizeDialog = group.readEntry("Size", QSize(800, 600));
    if (sizeDialog.isValid()) {
        resize(sizeDialog);
    }
}

void BalooDebugDialog::writeConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "BalooDebugDialog");
    group.writeEntry("Size", size());
}

void BalooDebugDialog::setAkonadiId(Akonadi::Item::Id akonadiId)
{
    d->mBalooDebugWidget->setAkonadiId(akonadiId);
}

void BalooDebugDialog::setSearchType(BalooDebugSearchPathComboBox::SearchType type)
{
    d->mBalooDebugWidget->setSearchType(type);
}

void BalooDebugDialog::doSearch()
{
    d->mBalooDebugWidget->doSearch();
}

void BalooDebugDialog::slotSaveAs()
{
    const QString filter = i18n("Text Files (*.txt);;All Files (*)");
    PimCommon::Util::saveTextAs(d->mBalooDebugWidget->plainText(), filter, this);
}

