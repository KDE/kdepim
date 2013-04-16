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

#include "previewpage.h"
#include "messageviewer/viewer.h"

#include <KPushButton>
#include <KLocale>

#include <QVBoxLayout>

PreviewPage::PreviewPage(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *lay = new QVBoxLayout;
    mViewer = new MessageViewer::Viewer(this);
    lay->addWidget(mViewer);
    mUpdate = new KPushButton(i18n("Update view"));
    connect(mUpdate, SIGNAL(clicked(bool)),SLOT(slotUpdateViewer()));
    lay->addWidget(mUpdate);
    setLayout(lay);
}

PreviewPage::~PreviewPage()
{
}

void PreviewPage::slotUpdateViewer()
{
    //TODO load a default message.
}

#include "previewpage.moc"
