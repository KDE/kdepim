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

#ifndef SIEVESCRIPTTABWIDGET_H
#define SIEVESCRIPTTABWIDGET_H

#include <KTabWidget>

namespace KSieveUi {
class SieveScriptTabWidget : public KTabWidget
{
    Q_OBJECT
public:
    explicit SieveScriptTabWidget(QWidget *parent = 0);
    ~SieveScriptTabWidget();

private Q_SLOTS:
    void slotTabContextMenuRequest(const QPoint &);
};
}

#endif // SIEVESCRIPTTABWIDGET_H
