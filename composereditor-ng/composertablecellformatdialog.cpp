/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

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

#include "composertablecellformatdialog.h"

#include <KLocale>
#include <KColorButton>

#include <QWebElement>
#include <QVBoxLayout>
#include <QCheckBox>

namespace ComposerEditorNG {

class ComposerTableCellFormatDialogPrivate
{
public:
    ComposerTableCellFormatDialogPrivate(const QWebElement& element, ComposerTableCellFormatDialog *qq)
        :webElement(element)
        ,q(qq)
    {
        q->setButtons( KDialog::Ok | KDialog::Cancel );
        q->setCaption( i18n( "Edit Cell Format" ) );

        QVBoxLayout *layout = new QVBoxLayout( q->mainWidget() );

        //layout->addWidget( linkLocation );
        q->connect(q,SIGNAL(okClicked()),q,SLOT(_k_slotOkClicked()));

    }

    void _k_slotOkClicked();

    QWebElement webElement;
    //TODO
    KColorButton *backgroundColor;
    QCheckBox *useBackgroundColor;

    ComposerTableCellFormatDialog *q;
};

void ComposerTableCellFormatDialogPrivate::_k_slotOkClicked()
{
    if(!webElement.isNull()) {
        //TODO
    }
    q->accept();
}

ComposerTableCellFormatDialog::ComposerTableCellFormatDialog(const QWebElement& element, QWidget *parent)
    :KDialog(parent), d(new ComposerTableCellFormatDialogPrivate(element, this))
{
}

ComposerTableCellFormatDialog::~ComposerTableCellFormatDialog()
{
    delete d;
}

}

#include "composertablecellformatdialog.moc"
