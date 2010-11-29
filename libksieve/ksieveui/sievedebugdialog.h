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

#ifndef KSIEVEUI_SIEVEDEBUGDIALOG_H
#define KSIEVEUI_SIEVEDEBUGDIALOG_H

#include "ksieveui_export.h"

#include <kdialog.h>
#include <kurl.h>

class KTextEdit;

class QString;
class QStringList;

namespace KManageSieve {
  class SieveJob;
}

namespace KSieveUi
{

/**
 * Diagnostic info for Sieve. Only compiled when debug is enabled, it is
 * not useful enough for non-developers to have this in releases.
 */
class KSIEVEUI_EXPORT SieveDebugDialog : public KDialog
{
    Q_OBJECT

  public:
    SieveDebugDialog( QWidget *parent = 0 );
    virtual ~SieveDebugDialog();

  protected:
    void handlePutResult( KManageSieve::SieveJob *job, bool success, bool );

  signals:
    void result( bool success );

  protected slots:
    void slotGetScript( KManageSieve::SieveJob *job, bool success, const QString &script, bool active );
    void slotGetScriptList( KManageSieve::SieveJob *job, bool success, const QStringList &scriptList, const QString &activeScript );

    void slotDialogOk();
    void slotPutActiveResult( KManageSieve::SieveJob*, bool );
    void slotPutInactiveResult( KManageSieve::SieveJob*, bool );
    void slotDiagNextAccount();
    void slotDiagNextScript();

  protected:
    KManageSieve::SieveJob *mSieveJob;
    KUrl mUrl;

    KTextEdit *mEdit;

    QStringList mResourceIdentifier;
    QStringList mScriptList;
};

}

#endif
