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

        QHBoxLayout *hbox = new QHBoxLayout;
        useBackgroundColor = new QCheckBox( i18n( "Background Color:" ) );
        hbox->addWidget(useBackgroundColor);
        backgroundColor = new KColorButton;
        backgroundColor->setEnabled(false);
        hbox->addWidget(backgroundColor);

        layout->addLayout(hbox);
        q->connect(useBackgroundColor,SIGNAL(toggled(bool)),backgroundColor,SLOT(setEnabled(bool)));

        q->connect(q,SIGNAL(okClicked()),q,SLOT(_k_slotOkClicked()));

        if(!webElement.isNull()) {
            if(webElement.hasAttribute(QLatin1String("bgcolor"))) {
                useBackgroundColor->setChecked(true);
                const QColor color = QColor(webElement.attribute(QLatin1String("bgcolor")));
                backgroundColor->setColor(color);
            }
        }
    }

    void _k_slotOkClicked();

    QWebElement webElement;
    KColorButton *backgroundColor;
    QCheckBox *useBackgroundColor;

    ComposerTableCellFormatDialog *q;
};

void ComposerTableCellFormatDialogPrivate::_k_slotOkClicked()
{
    if(!webElement.isNull()) {
        if(useBackgroundColor->isChecked()) {
            webElement.setAttribute(QLatin1String("bgcolor"),backgroundColor->color().name());
        } else {
            webElement.removeAttribute(QLatin1String("bgcolor"));
        }
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
