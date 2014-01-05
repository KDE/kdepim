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

#ifndef GRAMMARCOMBOBOXLANGUAGE_H
#define GRAMMARCOMBOBOXLANGUAGE_H

#include "grammar_export.h"

#include <KComboBox>

namespace Grammar
{
class GrammarComboBoxLanguagePrivate;
class GRAMMAR_EXPORT GrammarComboBoxLanguage : public KComboBox
{
    Q_OBJECT
public:
    explicit GrammarComboBoxLanguage(QWidget *parent = 0);
    ~GrammarComboBoxLanguage();

    void setCurrentLanguage(const QString &lang);
    QString currentLanguage() const;

Q_SIGNALS:
    void languageChanged(const QString &language);

private:
    void reloadList();
    friend class GrammarComboBoxLanguagePrivate;
    GrammarComboBoxLanguagePrivate * const d;
    Q_PRIVATE_SLOT(d, void slotLanguageChanged(int) )
};
}

#endif // GRAMMARCOMBOBOXLANGUAGE_H
