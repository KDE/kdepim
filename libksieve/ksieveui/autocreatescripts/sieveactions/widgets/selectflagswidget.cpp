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

#include "selectflagswidget.h"

#include <KLineEdit>

#include <QPushButton>
#include <QHBoxLayout>


using namespace KSieveUi;

SelectFlagsWidget::SelectFlagsWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *lay = new QHBoxLayout;
    mEdit = new KLineEdit;
    mEdit->setReadOnly(true);
    lay->addWidget(mEdit);
    QPushButton *selectFlags = new QPushButton;
    connect(selectFlags, SIGNAL(clicked(bool)), this, SLOT(slotSelectFlags()));
    lay->addWidget(selectFlags);
    setLayout(lay);
}

SelectFlagsWidget::~SelectFlagsWidget()
{
}

void SelectFlagsWidget::slotSelectFlags()
{
    //TODO
}

QString SelectFlagsWidget::code() const
{
    //TODO
    return QString();
}

#include "selectflagswidget.moc"
