/*
  sievedebugdialog.h

  Copyright (c) 2005 Martijn Klingens <klingens@kde.org>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License,
  version 2.0, as published by the Free Software Foundation.
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software Foundation,
  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef KSIEVE_KSIEVEUI_SIEVEDEBUGDIALOG_H
#define KSIEVE_KSIEVEUI_SIEVEDEBUGDIALOG_H

#include "ksieveui_export.h"

#include <QDialog>
#include <QUrl>

class QStringList;

namespace KManageSieve
{
class SieveJob;
}
namespace PimCommon
{
class PlainTextEditorWidget;
}
namespace KSieveUi
{

/**
 * Diagnostic info for Sieve. Only compiled when debug is enabled, it is
 * not useful enough for non-developers to have this in releases.
 */
class KSIEVEUI_EXPORT SieveDebugDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SieveDebugDialog(QWidget *parent = 0);
    ~SieveDebugDialog();

Q_SIGNALS:
    void result(bool success);

private Q_SLOTS:
    void slotGetScript(KManageSieve::SieveJob *job, bool success, const QString &script, bool active);
    void slotGetScriptList(KManageSieve::SieveJob *job, bool success, const QStringList &scriptList, const QString &activeScript);

    void slotDiagNextAccount();
    void slotDiagNextScript();

private:
    void writeConfig();
    void readConfig();
    KManageSieve::SieveJob *mSieveJob;
    QUrl mUrl;

    PimCommon::PlainTextEditorWidget *mEdit;
    QStringList mResourceIdentifier;
    QStringList mScriptList;
};

}

#endif
