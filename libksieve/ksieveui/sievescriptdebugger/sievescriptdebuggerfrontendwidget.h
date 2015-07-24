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

#ifndef SIEVESCRIPTDEBUGGERFRONTENDWIDGET_H
#define SIEVESCRIPTDEBUGGERFRONTENDWIDGET_H

#include <QWidget>
#include "ksieveui_export.h"

namespace PimCommon
{
class PlainTextEditorWidget;
}
class KUrlRequester;
class QPushButton;
class QProcess;
namespace KSieveUi
{
class SieveTextEditWidget;
class SieveScriptDebuggerWarning;
class KSIEVEUI_EXPORT SieveScriptDebuggerFrontEndWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SieveScriptDebuggerFrontEndWidget(QWidget *parent = Q_NULLPTR);
    ~SieveScriptDebuggerFrontEndWidget();

    QString script() const;
    void setScript(const QString &script);

private Q_SLOTS:
    void slotDebugScript();
    void slotEmailChanged(const QString &text);
    void slotReadStandardOutput();
    void slotReadErrorOutput();
    void slotDebugFinished();

private:
    KSieveUi::SieveTextEditWidget *mSieveTextEditWidget;
    KSieveUi::SieveScriptDebuggerWarning *mSieveScriptDebuggerWarning;
    PimCommon::PlainTextEditorWidget *mSieveTestResult;
    KUrlRequester *mEmailPath;
    QPushButton *mDebugScript;
    QProcess *mProcess;
};
}
#endif // SIEVESCRIPTDEBUGGERFONTENDWIDGET_H
