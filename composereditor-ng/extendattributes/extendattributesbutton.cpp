/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include "extendattributesbutton.h"

#include <KLocalizedString>

#include <QPointer>
#include <QWebElement>


namespace ComposerEditorNG
{

class ExtendAttributesButtonPrivate
{
public:
    ExtendAttributesButtonPrivate(const QWebElement &element, ExtendAttributesDialog::ExtendType type, ExtendAttributesButton *qq)
        : webElement(element), extendType(type), q(qq)
    {
        q->setText(i18n("Advanced"));
        q->connect(q, SIGNAL(clicked(bool)), q, SLOT(_k_slotClicked()));
    }
    void _k_slotClicked();

    QWebElement webElement;
    ExtendAttributesDialog::ExtendType extendType;
    ExtendAttributesButton *q;
};

void ExtendAttributesButtonPrivate::_k_slotClicked()
{
    QPointer<ExtendAttributesDialog> dlg = new ExtendAttributesDialog(webElement, extendType, q );
    if (dlg->exec()) {
        Q_EMIT q->webElementChanged();
    }
    delete dlg;
}

ExtendAttributesButton::ExtendAttributesButton(const QWebElement &element, ExtendAttributesDialog::ExtendType type, QWidget *parent)
    : QPushButton(parent), d(new ExtendAttributesButtonPrivate(element, type, this))
{
}

ExtendAttributesButton::~ExtendAttributesButton()
{
    delete d;
}
}

#include "moc_extendattributesbutton.cpp"
