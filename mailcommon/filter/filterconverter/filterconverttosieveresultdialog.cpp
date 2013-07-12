/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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


#include "filterconverttosieveresultdialog.h"

#include <KTextEdit>
#include <KLocale>

#include <QHBoxLayout>

using namespace MailCommon;

FilterConvertToSieveResultDialog::FilterConvertToSieveResultDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Convert to sieve script" ) );
    setButtons( Ok|Cancel );
    setDefaultButton( Ok );
    setModal( true );

    QWidget *mainWidget = new QWidget( this );
    QHBoxLayout *mainLayout = new QHBoxLayout( mainWidget );

    mainLayout->setSpacing( KDialog::spacingHint() );
    mainLayout->setMargin( KDialog::marginHint() );
    mEditor = new KTextEdit;
    mEditor->setAcceptRichText(false);
    mainLayout->addWidget(mEditor);
    setMainWidget( mainWidget );
}

FilterConvertToSieveResultDialog::~FilterConvertToSieveResultDialog()
{

}

void FilterConvertToSieveResultDialog::setCode(const QString &code)
{
    mEditor->setPlainText(code);
}

#include "filterconverttosieveresultdialog.moc"
