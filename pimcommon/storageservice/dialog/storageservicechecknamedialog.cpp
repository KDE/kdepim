/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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


#include "storageservicechecknamedialog.h"

#include <KLocalizedString>

#include <QLineEdit>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

using namespace PimCommon;

StorageServiceCheckNameDialog::StorageServiceCheckNameDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption(i18n("New name"));
    setButtons(Ok|Cancel);
    QVBoxLayout *lay = new QVBoxLayout;
    QWidget *w = new QWidget;
    w->setLayout(lay);
    setMainWidget(w);

    mInfo = new QLabel(i18n("Some characters are not allowed."));
    lay->addWidget(mInfo);

    QHBoxLayout *hbox = new QHBoxLayout;
    QLabel *lab = new QLabel(i18n("New name:"));
    hbox->addWidget(lab);
    mName = new QLineEdit;
    hbox->addWidget(mName);
    lay->addLayout(hbox);
    connect(mName, SIGNAL(textChanged(QString)), this, SLOT(slotNameChanged(QString)));
}

StorageServiceCheckNameDialog::~StorageServiceCheckNameDialog()
{

}

void StorageServiceCheckNameDialog::slotNameChanged(const QString &text)
{
    if (text.isEmpty() || text.contains(mRegExp) || text == QLatin1String(".") || text == QLatin1String("..")) {
        enableButtonOk(false);
        return;
    }
    enableButtonOk(true);
}

void StorageServiceCheckNameDialog::setDisallowedSymbols(const QRegExp &regExp)
{
    mRegExp = regExp;
}

void StorageServiceCheckNameDialog::setDisallowedSymbolsStr(const QString &str)
{
    if (!str.isEmpty())
        mInfo->setText(i18n("Some characters (%1) are not allowed.", str));
}

void StorageServiceCheckNameDialog::setOldName(const QString &name)
{
    mName->setText(name);
}

QString StorageServiceCheckNameDialog::newName() const
{
    return mName->text();
}


#include "moc_storageservicechecknamedialog.cpp"
