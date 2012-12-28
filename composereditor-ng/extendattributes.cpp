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

#include "extendattributes.h"

#include <KSeparator>
#include <KLocale>

#include <QWebElement>
#include <QVBoxLayout>

namespace ComposerEditorNG {

class ExtendAttributesPrivate
{
public:
    ExtendAttributesPrivate(const QWebElement& element, ExtendAttributes::ExtendType extendType, ExtendAttributes *qq)
        : webElement(element), type(extendType), q(qq)
    {
        q->setCaption( i18n( "Extend Attribute" ) );
        q->setButtons( KDialog::Ok | KDialog::Cancel );
        QWidget *page = new QWidget( q );
        q->setMainWidget( page );

        QVBoxLayout *lay = new QVBoxLayout( page );

        KSeparator *sep = new KSeparator;
        lay->addWidget( sep );

        q->connect(q,SIGNAL(okClicked()),q,SLOT(_k_slotOkClicked()));
    }
    void _k_slotOkClicked();

    QWebElement webElement;
    ExtendAttributes::ExtendType type;
    ExtendAttributes *q;
};

void ExtendAttributesPrivate::_k_slotOkClicked()
{
    //TODO
    q->accept();
}

ExtendAttributes::ExtendAttributes(const QWebElement &element, ExtendType type, QWidget *parent)
    : KDialog(parent), d(new ExtendAttributesPrivate(element, type, this))
{
}

ExtendAttributes::~ExtendAttributes()
{
    delete d;
}

}

#include "extendattributes.moc"
