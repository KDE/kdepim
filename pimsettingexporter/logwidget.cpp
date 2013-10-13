/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>

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

#include "logwidget.h"

#include "libkdepim/widgets/customlogwidget.h"

#include <QHBoxLayout>
#include <QListWidget>


LogWidget::LogWidget(QWidget * parent)
    :QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout;
    mCustomLogWidget = new KPIM::CustomLogWidget;
    layout->addWidget(mCustomLogWidget);
    setLayout(layout);
}

LogWidget::~LogWidget()
{

}

void LogWidget::clear()
{
    mCustomLogWidget->clear();
}

void LogWidget::addInfoLogEntry( const QString &log )
{
    mCustomLogWidget->addInfoLogEntry(log);
}

void LogWidget::addErrorLogEntry( const QString &log )
{
    mCustomLogWidget->addErrorLogEntry(log);
}

QString LogWidget::toHtml() const
{
    return mCustomLogWidget->toHtml();
}
