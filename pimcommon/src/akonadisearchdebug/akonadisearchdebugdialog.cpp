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

#include "akonadisearchdebugdialog.h"
#include "akonadisearchdebugwidget.h"
#include "util/pimutil.h"

#include <KLocalizedString>
#include <QVBoxLayout>
#include <KSharedConfig>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>

using namespace PimCommon;

class PimCommon::AkonadiSearchDebugDialogPrivate
{
public:
    AkonadiSearchDebugDialogPrivate()
        : mBalooDebugWidget(Q_NULLPTR)
    {

    }

    AkonadiSearchDebugWidget *mBalooDebugWidget;
};

AkonadiSearchDebugDialog::AkonadiSearchDebugDialog(QWidget *parent)
    : QDialog(parent),
      d(new PimCommon::AkonadiSearchDebugDialogPrivate)
{
    //Don't translate it's just a dialog to debug
    setWindowTitle(QStringLiteral("Debug Akonadi Search"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    QPushButton *user1Button = new QPushButton;
    buttonBox->addButton(user1Button, QDialogButtonBox::ActionRole);
    //Don't translate it.
    user1Button->setText(QStringLiteral("Save As..."));
    connect(user1Button, &QPushButton::clicked, this, &AkonadiSearchDebugDialog::slotSaveAs);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &AkonadiSearchDebugDialog::reject);
    d->mBalooDebugWidget = new AkonadiSearchDebugWidget(this);
    d->mBalooDebugWidget->setObjectName(QStringLiteral("akonadisearchdebugwidget"));
    mainLayout->addWidget(d->mBalooDebugWidget);
    mainLayout->addWidget(buttonBox);
    readConfig();
}

AkonadiSearchDebugDialog::~AkonadiSearchDebugDialog()
{
    writeConfig();
    delete d;
}

void AkonadiSearchDebugDialog::readConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "AkonadiSearchDebugDialog");
    const QSize sizeDialog = group.readEntry("Size", QSize(800, 600));
    if (sizeDialog.isValid()) {
        resize(sizeDialog);
    }
}

void AkonadiSearchDebugDialog::writeConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "AkonadiSearchDebugDialog");
    group.writeEntry("Size", size());
}

void AkonadiSearchDebugDialog::setAkonadiId(Akonadi::Item::Id akonadiId)
{
    d->mBalooDebugWidget->setAkonadiId(akonadiId);
}

void AkonadiSearchDebugDialog::setSearchType(AkonadiSearchDebugSearchPathComboBox::SearchType type)
{
    d->mBalooDebugWidget->setSearchType(type);
}

void AkonadiSearchDebugDialog::doSearch()
{
    d->mBalooDebugWidget->doSearch();
}

void AkonadiSearchDebugDialog::slotSaveAs()
{
    const QString filter = i18n("Text Files (*.txt);;All Files (*)");
    PimCommon::Util::saveTextAs(d->mBalooDebugWidget->plainText(), filter, this);
}

