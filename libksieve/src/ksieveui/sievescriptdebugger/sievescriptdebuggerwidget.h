/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#ifndef SIEVESCRIPTDEBUGGERWIDGET_H
#define SIEVESCRIPTDEBUGGERWIDGET_H

#include <QWidget>
#include "ksieveui_export.h"
class QStackedWidget;
class QLabel;
namespace KSieveUi
{
class SieveScriptDebuggerFrontEndWidget;
class KSIEVEUI_EXPORT SieveScriptDebuggerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SieveScriptDebuggerWidget(QWidget *parent = Q_NULLPTR);
    ~SieveScriptDebuggerWidget();

    void setScript(const QString &script);
    QString script() const;

    bool canAccept() const;

private:
    void checkSieveTestApplication();
    QStackedWidget *mStackedWidget;
    SieveScriptDebuggerFrontEndWidget *mSieveScriptFrontEnd;
    QLabel *mSieveNoExistingFrontEnd;
};
}
#endif // SIEVESCRIPTDEBUGGERWIDGET_H
