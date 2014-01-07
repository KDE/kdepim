/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#include "sieveeditorpagewidget.h"

#include <kmanagesieve/sievejob.h>

SieveEditorPageWidget::SieveEditorPageWidget(QWidget *parent)
    : KSieveUi::SieveEditor(parent),
      mWasActive(false)
{
}

SieveEditorPageWidget::~SieveEditorPageWidget()
{

}

void SieveEditorPageWidget::loadScript(const KUrl &url, const QStringList &capabilities)
{
    mCurrentURL = url;
    setSieveCapabilities(capabilities);
    KManageSieve::SieveJob * job = KManageSieve::SieveJob::get( url );
    connect( job, SIGNAL(result(KManageSieve::SieveJob*,bool,QString,bool)),
             this, SLOT(slotGetResult(KManageSieve::SieveJob*,bool,QString,bool)) );
}

KUrl SieveEditorPageWidget::currentUrl() const
{
    return mCurrentURL;
}

void SieveEditorPageWidget::slotGetResult( KManageSieve::SieveJob *, bool success, const QString & script, bool isActive )
{
    if ( !success )
        return;
    setScriptName( mCurrentURL.fileName() );
    setScript( script );
    mWasActive = isActive;
}
