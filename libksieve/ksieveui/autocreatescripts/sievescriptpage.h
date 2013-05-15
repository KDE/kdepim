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

#ifndef SIEVESCRIPTPAGE_H
#define SIEVESCRIPTPAGE_H

#include <QWidget>
#include "sievescriptblockwidget.h"

namespace KSieveUi {
class SieveScriptBlockWidget;
class SieveScriptTabWidget;
class SieveIncludeWidget;
class SieveScriptPage : public QWidget
{
    Q_OBJECT
public:
    explicit SieveScriptPage(QWidget *parent = 0);
    ~SieveScriptPage();

    void generatedScript(QString &script, QStringList &requires);


private Q_SLOTS:
    void slotAddNewBlock(QWidget* widget,KSieveUi::SieveWidgetPageAbstract::PageType type);
    void slotCloseTab(int);

private:
    bool hasAnElseBlock() const;
    QString blockName(SieveWidgetPageAbstract::PageType type) const;
    SieveScriptBlockWidget *createScriptBlock(KSieveUi::SieveWidgetPageAbstract::PageType type);
    SieveScriptTabWidget *mTabWidget;
    SieveIncludeWidget *mIncludeWidget;
};
}

#endif // SIEVESCRIPTPAGE_H
