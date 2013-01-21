/*
  Copyright (c) 2013 Montel Laurent <montel.org>

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

#include "composerlistdialog.h"
#include "extendattributes/extendattributesbutton.h"
#include "helper/listhelper_p.h"

#include <KLocale>
#include <KSeparator>

#include <QVBoxLayout>
#include <QWebElement>
#include <QComboBox>
#include <QLabel>

namespace ComposerEditorNG
{

class ComposerListDialogPrivate
{
public:
    ComposerListDialogPrivate(const QWebElement& element, ComposerListDialog *qq)
        : webElement(element),
          q(qq)
    {
        initialize();
    }
    void _k_slotOkClicked();
    void _k_slotWebElementChanged();

    enum ListType {
        None = 0,
        Bullet = 1,
        Numbered = 2,
        Definition = 3
    };

    void initialize();
    void updateSettings();
    void updateListHtml();
    QWebElement webElement;
    QComboBox *listType;
    ComposerListDialog *q;
};

void ComposerListDialogPrivate::initialize()
{
    q->setButtons( KDialog::Ok | KDialog::Cancel );
    q->setCaption( i18n( "Edit List" ) );

    QVBoxLayout *vbox = new QVBoxLayout(q->mainWidget());

    QLabel *lab = new QLabel(i18n("List Type"));
    vbox->addWidget(lab);

    listType = new QComboBox;
    vbox->addWidget(listType);
    listType->addItem(i18n("None"), None);
    listType->addItem(i18n("Bullet List"), Bullet);
    listType->addItem(i18n("Numbered List"), Numbered);
    listType->addItem(i18n("Definition List"), Definition);

    KSeparator *sep = 0;
    if (!webElement.isNull()) {
        sep = new KSeparator;
        vbox->addWidget( sep );
        //TODO customize
        ExtendAttributesButton *button = new ExtendAttributesButton(webElement,ExtendAttributesDialog::ListUL,q);
        q->connect(button, SIGNAL(webElementChanged()), q, SLOT(_k_slotWebElementChanged()));
        vbox->addWidget( button );
    }

    sep = new KSeparator;
    vbox->addWidget( sep );

    q->connect(q, SIGNAL(okClicked()), q, SLOT(_k_slotOkClicked()));
}

void ComposerListDialogPrivate::updateSettings()
{
    if (!webElement.isNull()) {
        //TODO
    }
}

void ComposerListDialogPrivate::updateListHtml()
{
    /*
    QWebElement e = ListHelper::olElement(webElement);
    e.addClass(QLatin1String("UL"));
    */
    //TODO
}

void ComposerListDialogPrivate::_k_slotWebElementChanged()
{
    //TODO
}

void ComposerListDialogPrivate::_k_slotOkClicked()
{
    if (!webElement.isNull()) {
        updateListHtml();
    }
    q->accept();
}

ComposerListDialog::ComposerListDialog(const QWebElement& element, QWidget *parent)
    : KDialog(parent), d(new ComposerListDialogPrivate(element, this))
{
}

ComposerListDialog::~ComposerListDialog()
{
    delete d;
}

}

#include "composerlistdialog.moc"
