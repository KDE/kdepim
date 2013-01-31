/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>
  
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

#include "selectspecialchar.h"
#include <KCharSelect>
#include <KLocale>
#include <QHBoxLayout>

class SelectSpecialCharPrivate {
public:
    SelectSpecialCharPrivate(SelectSpecialChar *qq)
        : q( qq )
    {
        q->setCaption( i18n("Select Special Characters") );
        q->setButtons( KDialog::Ok|KDialog::Cancel|KDialog::User1 );
        q->setButtonText(KDialog::User1,i18n("Select"));
        QWidget *page = new QWidget( q );
        q->setMainWidget( page );
        QHBoxLayout *lay = new QHBoxLayout(page);
        mCharSelect = new KCharSelect(q, 0, KCharSelect::CharacterTable|KCharSelect::BlockCombos);
        q->connect(mCharSelect,SIGNAL(charSelected(QChar)),q,SIGNAL(charSelected(QChar)));
        lay->addWidget(mCharSelect);
        q->connect(q,SIGNAL(user1Clicked()),q,SLOT(_k_slotInsertChar()));
        q->connect(q,SIGNAL(okClicked()),q,SLOT(_k_slotInsertChar()));
    }

    void _k_slotInsertChar();

    KCharSelect *mCharSelect;
    SelectSpecialChar *q;
};

void SelectSpecialCharPrivate::_k_slotInsertChar()
{
    Q_EMIT q->charSelected(mCharSelect->currentChar());
}

SelectSpecialChar::SelectSpecialChar(QWidget *parent)
    :KDialog(parent), d(new SelectSpecialCharPrivate(this))
{
}

SelectSpecialChar::~SelectSpecialChar()
{
    delete d;
}

void SelectSpecialChar::showSelectButton(bool show)
{
    if(show)
        setButtons( Ok|Cancel|User1 );
    else
        setButtons( Ok|Cancel );
}

void SelectSpecialChar::setCurrentChar(const QChar &c)
{
    d->mCharSelect->setCurrentChar(c);
}

QChar SelectSpecialChar::currentChar() const
{
    return d->mCharSelect->currentChar();
}

void SelectSpecialChar::autoInsertChar()
{
    connect(d->mCharSelect,SIGNAL(charSelected(QChar)),SLOT(accept()));
}

void SelectSpecialChar::setOkButtonText(const QString& text)
{
    setButtonText(KDialog::User1,text);
}

#include "selectspecialchar.moc"
