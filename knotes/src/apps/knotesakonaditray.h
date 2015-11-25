/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#ifndef KNOTESAKONADITRAY_H
#define KNOTESAKONADITRAY_H

#include <KStatusNotifierItem>
#include <QIcon>
class KNotesAkonadiTray : public KStatusNotifierItem
{
    Q_OBJECT
public:
    explicit KNotesAkonadiTray(QWidget *parent = Q_NULLPTR);
    ~KNotesAkonadiTray();

    void updateNumberOfNotes(int number);

private Q_SLOTS:
    void slotGeneralPaletteChanged();

private:
    QIcon mIcon;
    QColor mTextColor;
};

#endif // KNOTESAKONADITRAY_H
