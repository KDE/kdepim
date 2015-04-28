/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "invalidfilterdialog.h"
#include "invalidfilterwidget.h"
#include "invalidfilterinfowidget.h"
#include <KConfigGroup>
#include <KSharedConfig>
#include <QVBoxLayout>
#include <KLocalizedString>
#include <QDialogButtonBox>
#include <QPushButton>

using namespace MailCommon;

InvalidFilterDialog::InvalidFilterDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18n("Invalid Filters"));
    setWindowIcon(QIcon::fromTheme(QLatin1String("kmail")));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    okButton->setDefault(true);
    setModal(true);
    okButton->setText(i18n("Discard"));

    
    QWidget *w = new QWidget(this);
    QVBoxLayout *vbox = new QVBoxLayout;
    w->setLayout(vbox);
    mInvalidFilterWidget = new InvalidFilterWidget(this);
    mInvalidFilterWidget->setObjectName(QLatin1String("invalid_filter_widget"));
    vbox->addWidget(mInvalidFilterWidget);

    mInvalidFilterInfoWidget = new InvalidFilterInfoWidget(this);
    mInvalidFilterInfoWidget->setObjectName(QLatin1String("invalid_filter_infowidget"));
    vbox->addWidget(mInvalidFilterInfoWidget);
    connect(mInvalidFilterWidget, SIGNAL(showDetails(QString)), mInvalidFilterInfoWidget, SLOT(slotShowDetails(QString)));
    connect(mInvalidFilterWidget, SIGNAL(hideInformationWidget()), mInvalidFilterInfoWidget, SLOT(animatedHide()));
    mainLayout->addWidget(w);
    mainLayout->addWidget(buttonBox);
    readConfig();
}

InvalidFilterDialog::~InvalidFilterDialog()
{
    writeConfig();
}

void InvalidFilterDialog::setInvalidFilters(const QVector<InvalidFilterInfo> &lst)
{
    mInvalidFilterWidget->setInvalidFilters(lst);
}

void InvalidFilterDialog::writeConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "InvalidFilterDialog");
    group.writeEntry("Size", size());
}

void InvalidFilterDialog::readConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "InvalidFilterDialog");
    const QSize sizeDialog = group.readEntry("Size", QSize(300, 350));
    if (sizeDialog.isValid()) {
        resize(sizeDialog);
    }
}

