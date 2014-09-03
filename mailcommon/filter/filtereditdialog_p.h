/*
    Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
    Copyright (c) 2010 Andras Mantia <andras@kdab.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef MAILCOMMON_FILTEREDITDIALOG_P_H
#define MAILCOMMON_FILTEREDITDIALOG_P_H

#include <kdialog.h>

class Ui_FilterConfigWidget;

namespace MailCommon
{

class MailFilter;
class SearchPatternEdit;
class FilterActionWidgetLister;

class FilterEditDialog : public KDialog
{
    Q_OBJECT

public:
    explicit FilterEditDialog(QWidget *parent = 0);
    virtual ~FilterEditDialog();

    void load(int index);
    void save();

private:
    Ui_FilterConfigWidget *mUi;
    MailFilter *mFilter;
    SearchPatternEdit *mPatternEdit;
    FilterActionWidgetLister *mActionLister;
};

}

#endif
