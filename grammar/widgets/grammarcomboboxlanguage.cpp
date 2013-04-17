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


#include "grammarcomboboxlanguage.h"

namespace Grammar {
class GrammarComboBoxLanguagePrivate {
public:
    GrammarComboBoxLanguagePrivate(GrammarComboBoxLanguage *qq)
        : q(qq)
    {

    }

    void slotLanguageChanged(int index)
    {
        Q_EMIT q->languageChanged( q->itemData( index ).toString() );
    }

    GrammarComboBoxLanguage *q;
};


GrammarComboBoxLanguage::GrammarComboBoxLanguage(QWidget *parent)
    : KComboBox(parent), d(new GrammarComboBoxLanguagePrivate(this))
{
    reloadList();
    connect( this, SIGNAL(activated(int)), this, SLOT(slotLanguageChanged(int)) );
}

GrammarComboBoxLanguage::~GrammarComboBoxLanguage()
{
    delete d;
}

void GrammarComboBoxLanguage::reloadList()
{
    clear();
    //TODO
}

void GrammarComboBoxLanguage::setCurrentLanguage(const QString &lang)
{
    //TODO
}

}

#include "grammarcomboboxlanguage.moc"
