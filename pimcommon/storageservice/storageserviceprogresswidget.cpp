/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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


#include "storageserviceprogresswidget.h"

#include <QProgressBar>
#include <QHBoxLayout>

using namespace PimCommon;

StorageServiceProgressWidget::StorageServiceProgressWidget(QWidget *parent)
    : QFrame(parent)
{
    QHBoxLayout *box = new QHBoxLayout( this );
    box->setMargin(0);
    box->setSpacing(0);
    mProgressBar = new QProgressBar;
    mProgressBar->setMinimum(0);
    mProgressBar->setMaximum(100);
    box->addWidget(mProgressBar);
}

StorageServiceProgressWidget::~StorageServiceProgressWidget()
{

}

void StorageServiceProgressWidget::setProgressValue(int val)
{
    mProgressBar->setValue(val);
}
