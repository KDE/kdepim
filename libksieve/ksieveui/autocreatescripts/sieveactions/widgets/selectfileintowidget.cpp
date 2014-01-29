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

#include "selectfileintowidget.h"

#include <KLineEdit>
#include <KLocalizedString>

#include <QPushButton>
#include <QHBoxLayout>

using namespace KSieveUi;

SelectFileIntoDialog::SelectFileIntoDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Select folder" ) );
    setButtons( Ok|Cancel );
    setButtonFocus( Ok );

    //TODO get list of folder for specific imap account.
}

SelectFileIntoDialog::~SelectFileIntoDialog()
{
}

QString SelectFileIntoDialog::selectedFolder() const
{
    //TODO
    return QString();
}


SelectFileIntoWidget::SelectFileIntoWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    mLineEdit = new KLineEdit;
    connect(mLineEdit, SIGNAL(textChanged(QString)), this, SIGNAL(valueChanged()));
    lay->addWidget(mLineEdit);
    QPushButton *selectFileInfo = new QPushButton(i18n("..."));
    connect(selectFileInfo, SIGNAL(clicked(bool)), SLOT(slotSelectFolder()));
    lay->addWidget(selectFileInfo);
    setLayout(lay);
}

SelectFileIntoWidget::~SelectFileIntoWidget()
{

}

void SelectFileIntoWidget::slotSelectFolder()
{
    QPointer<SelectFileIntoDialog> dialog = new SelectFileIntoDialog;
    if (dialog->exec()) {
        mLineEdit->setText(dialog->selectedFolder());
    }
    delete dialog;
}

QString SelectFileIntoWidget::selectedFolder() const
{
    return mLineEdit->text();
}


#include "moc_selectfileintowidget.cpp"
