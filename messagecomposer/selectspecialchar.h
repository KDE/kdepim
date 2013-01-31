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

#ifndef SelectSpecialChar_H
#define SelectSpecialChar_H
#include "messagecomposer_export.h"

#include <KDialog>

class KCharSelect;
class SelectSpecialCharPrivate;
class MESSAGECOMPOSER_EXPORT SelectSpecialChar : public KDialog
{
    Q_OBJECT
public:
    explicit SelectSpecialChar(QWidget *parent);
    ~SelectSpecialChar();

    void setCurrentChar(const QChar &c);
    QChar currentChar() const;

    void setOkButtonText(const QString& text);

    void showSelectButton(bool show);

    /*
   * When we double click we call accept
   */
    void autoInsertChar();

Q_SIGNALS:
    void charSelected(const QChar&);

private:
    SelectSpecialCharPrivate * const d;
    friend class SelectSpecialCharPrivate;
    Q_PRIVATE_SLOT( d, void _k_slotInsertChar() )
};

#endif // SelectSpecialChar_H
