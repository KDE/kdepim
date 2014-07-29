/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include "adblockaddsubscriptiondialog.h"
#include "adblockutil.h"

#include <KLocalizedString>

#include <QComboBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <KConfigGroup>
#include <QPushButton>
#include <QVBoxLayout>

using namespace MessageViewer;
AdBlockAddSubscriptionDialog::AdBlockAddSubscriptionDialog(const QStringList &excludeList, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle( i18n("Add subscription") );
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    QWidget *w = new QWidget;
    QHBoxLayout *lay = new QHBoxLayout;
    QLabel *lab = new QLabel(i18n("Select List:"));
    lay->addWidget(lab);

    mListSubscription = new QComboBox;
    lay->addWidget(mListSubscription);

    w->setLayout(lay);
    mainLayout->addWidget(w);
    mainLayout->addWidget(buttonBox);
    initializeList(excludeList);
}

AdBlockAddSubscriptionDialog::~AdBlockAddSubscriptionDialog()
{

}

void AdBlockAddSubscriptionDialog::initializeList(const QStringList &excludeList)
{
    QMapIterator<QString, QString> i(MessageViewer::AdBlockUtil::listSubscriptions());
    while (i.hasNext()) {
        i.next();
        if (!excludeList.contains(i.key())) {
            mListSubscription->addItem(i.key(), i.value());
        }
    }
}

void AdBlockAddSubscriptionDialog::selectedList(QString &name, QString &url)
{
    name = mListSubscription->currentText();
    url = mListSubscription->itemData(mListSubscription->currentIndex()).toString();
}

