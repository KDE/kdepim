/*
    This file is part of Akonadi.

    Copyright (c) 2008 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#include "searchdialog.h"

#include <QLineEdit>
#include <KTextEdit>

#include <QLabel>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <KConfigGroup>
#include <QPushButton>
#include <QVBoxLayout>

SearchDialog::SearchDialog( QWidget *parent )
  : QDialog( parent )
{
  setWindowTitle( "Create Search" );
  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
  QVBoxLayout *mainLayout = new QVBoxLayout;
  setLayout(mainLayout);
  QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
  okButton->setDefault(true);
  okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
  connect(buttonBox, &QDialogButtonBox::accepted, this, &SearchDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &SearchDialog::reject);

  QGridLayout *layout = new QGridLayout;
  mainLayout->addLayout(layout);
  mainLayout->addWidget(buttonBox);
  mName = new QLineEdit;
  mName->setText( "My Search" );
  mEdit = new KTextEdit;
  mEdit->setAcceptRichText( false );
  mEdit->setWhatsThis( "Enter a SparQL query here" );

  layout->addWidget( new QLabel( "Search query name:" ), 0, 0 );
  layout->addWidget( mName, 0, 1 );
  layout->addWidget( mEdit, 1, 0, 1, 2 );
}

SearchDialog::~SearchDialog()
{
}

QString SearchDialog::searchName() const
{
  return mName->text();
}

QString SearchDialog::searchQuery() const
{
  return mEdit->toPlainText();
}
