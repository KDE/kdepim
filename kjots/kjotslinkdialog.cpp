//
//  kjots
//
//  Copyright (C) 2008 Stephen Kelly <steveire@gmail.com>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#include "kjotslinkdialog.h"

#include <QLabel>
#include <QGridLayout>
#include <QRadioButton>

#include <KLocale>
#include <KComboBox>
#include <KLineEdit>
#include <KUrl>
#include <KActionCollection>
#include <kdebug.h>

#include "KJotsSettings.h"
#include "kjotsentry.h"
#include "bookshelf.h"

KJotsLinkDialog::KJotsLinkDialog(QWidget *parent, Bookshelf *bookshelf) :
        KDialog(parent)
{
    setCaption(i18n("Manage Link"));
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);
    setModal(true);
    showButtonSeparator(true);
    mBookshelf = bookshelf;
    QAbstractItemModel* model = mBookshelf->model();

    QWidget *entries = new QWidget(this);

    QGridLayout *layout = new QGridLayout(entries);

    textLabel = new QLabel(i18n("Link Text:"), this);
    textLineEdit = new KLineEdit(this);
    textLineEdit->setClearButtonShown(true);
    linkUrlLabel = new QLabel(i18n("Link URL:"), this);
    linkUrlLineEdit = new KLineEdit(this);
    hrefCombo = new KComboBox(this);
    linkUrlLineEdit->setClearButtonShown(true);

    tree = new QTreeView();
    tree->setModel(model);
    tree->expandAll();
    tree->setColumnHidden(1, true);
    hrefCombo->setModel(mBookshelf->model());
    hrefCombo->setView(tree);

    QGridLayout* linkLayout = new QGridLayout();
    linkUrlLineEditRadioButton = new QRadioButton(entries);
    hrefComboRadioButton = new QRadioButton(entries);

    connect(linkUrlLineEditRadioButton, SIGNAL(toggled(bool)), linkUrlLineEdit, SLOT(setEnabled(bool)));
    connect(hrefComboRadioButton, SIGNAL(toggled(bool)), hrefCombo, SLOT(setEnabled(bool)));
    hrefCombo->setEnabled(false);
    linkUrlLineEditRadioButton->setChecked(true);

    linkLayout->addWidget(linkUrlLineEditRadioButton, 0, 0);
    linkLayout->addWidget(linkUrlLineEdit, 0, 1);
    linkLayout->addWidget(hrefComboRadioButton, 1, 0);
    linkLayout->addWidget(hrefCombo, 1, 1);

    layout->addWidget(textLabel, 0, 0);
    layout->addWidget(textLineEdit, 0, 1);
    layout->addWidget(linkUrlLabel, 1, 0);
    layout->addLayout( linkLayout, 1, 1 );

    setMainWidget(entries);

    textLineEdit->setFocus();
}

void KJotsLinkDialog::setLinkText(const QString &linkText)
{
    textLineEdit->setText(linkText);
    if (!linkText.trimmed().isEmpty())
        linkUrlLineEdit->setFocus();
}

void KJotsLinkDialog::setLinkUrl(const QString &linkUrl)
{
    if (KJotsEntry::isKJotsLink(linkUrl)) {
        kDebug() << KJotsEntry::idFromLinkUrl(linkUrl);

        KJotsEntry* item = mBookshelf->entryFromId(KJotsEntry::idFromLinkUrl(linkUrl));
        if ( item ) {
            QModelIndex index = hrefCombo->model()->index(0,1);
            index = index.sibling(index.row(), 0);
            index = index.sibling(index.row() + 1, 0);
            index = index.sibling(index.row(), 1);
            index = index.sibling(index.row(), 0);

            hrefCombo->view()->setCurrentIndex(index);
        }
        hrefComboRadioButton->setChecked(true);
    } else {
        linkUrlLineEdit->setText(linkUrl);
        linkUrlLineEditRadioButton->setChecked(true);
    }
}

QString KJotsLinkDialog::linkText() const
{
    return textLineEdit->text().trimmed();
}

QString KJotsLinkDialog::linkUrl() const
{

    if (hrefComboRadioButton->isChecked()){
        QModelIndex index = hrefCombo->view()->currentIndex();
        index = index.sibling(index.row(), 1);
        quint64 id = hrefCombo->model()->data(index).toULongLong();
        return KJotsEntry::kjotsLinkUrlFromId(id);
    } else {
        return linkUrlLineEdit->text();
    }

}
