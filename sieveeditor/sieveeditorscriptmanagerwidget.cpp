/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

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

#include "sieveeditorscriptmanagerwidget.h"
#include "sieveeditormanagesievewidget.h"

#include <QHBoxLayout>

SieveEditorScriptManagerWidget::SieveEditorScriptManagerWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->setMargin(0);
    setLayout(hbox);
    mTreeView = new SieveEditorManageSieveWidget;
    hbox->addWidget(mTreeView);
}

SieveEditorScriptManagerWidget::~SieveEditorScriptManagerWidget()
{

}

void SieveEditorScriptManagerWidget::addServerImap(const KUrl &url)
{
    if (!mUrls.contains(url))
        mUrls.append(url);
    //TODO
}

void SieveEditorScriptManagerWidget::slotCreateNewScript()
{
    //TODO
}

void SieveEditorScriptManagerWidget::slotDeleteScript()
{

}
