/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

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
class SieveForEveryPartWidget;
class SieveGlobalVariableWidget;

class SieveScriptPage : public QWidget
{
    Q_OBJECT
public:
    explicit SieveScriptPage(QWidget *parent = 0);
    ~SieveScriptPage();

    void generatedScript(QString &script, QStringList &requires);

    SieveIncludeWidget *includeWidget() const;
    SieveForEveryPartWidget *forEveryPartWidget() const;
    SieveGlobalVariableWidget *globalVariableWidget() const;
    SieveScriptBlockWidget *blockIfWidget() const;
    SieveScriptBlockWidget *addScriptBlock(KSieveUi::SieveWidgetPageAbstract::PageType type);


private Q_SLOTS:
    void slotAddNewBlock(QWidget* widget,KSieveUi::SieveWidgetPageAbstract::PageType type);
    void slotCloseTab(int);

private:
    SieveScriptBlockWidget *createScriptBlock(KSieveUi::SieveWidgetPageAbstract::PageType type);
    bool hasAnElseBlock() const;
    QString blockName(SieveWidgetPageAbstract::PageType type) const;

    SieveScriptTabWidget *mTabWidget;
    SieveIncludeWidget *mIncludeWidget;
    SieveForEveryPartWidget *mForEveryPartWidget;
    SieveGlobalVariableWidget *mGlobalVariableWidget;
    SieveScriptBlockWidget *mBlockIfWidget;
};
}

#endif // SIEVESCRIPTPAGE_H
