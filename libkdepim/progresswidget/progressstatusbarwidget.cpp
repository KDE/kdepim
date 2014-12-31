/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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


#include "progressstatusbarwidget.h"

#include "statusbarprogresswidget.h"
#include "progressdialog.h"

using namespace KPIM;

ProgressStatusBarWidget::ProgressStatusBarWidget(QWidget *alignWidget, QWidget *parent, unsigned int showTypeProgressItem)
    : QObject(parent)
{
    KPIM::ProgressDialog *progressDialog = new KPIM::ProgressDialog( alignWidget, parent );
    progressDialog->setShowTypeProgressItem(showTypeProgressItem);
    progressDialog->hide();

    mLittleProgress = new KPIM::StatusbarProgressWidget( progressDialog, alignWidget );
    mLittleProgress->setShowTypeProgressItem(showTypeProgressItem);
    mLittleProgress->show();
}

ProgressStatusBarWidget::~ProgressStatusBarWidget()
{

}

KPIM::StatusbarProgressWidget *ProgressStatusBarWidget::littleProgress() const
{
    return mLittleProgress;
}
