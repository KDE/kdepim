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

#ifndef SIEVEINCLUDEWIDGET_H
#define SIEVEINCLUDEWIDGET_H

#include <QWidget>
#include <libkdepim/kwidgetlister.h>

namespace KSieveUi {
class SieveIncludeActionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SieveIncludeActionWidget(QWidget *parent = 0);
    ~SieveIncludeActionWidget();
};

class SieveIncludeWidgetLister : public KPIM::KWidgetLister
{
    Q_OBJECT
public:
    explicit SieveIncludeWidgetLister(QWidget *parent = 0);
    ~SieveIncludeWidgetLister();

    void generatedScript(QString &script, QStringList &requires);
public Q_SLOTS:
    void slotAddWidget( QWidget *w );
    void slotRemoveWidget( QWidget *w );

protected:
    void clearWidget( QWidget *aWidget );
    QWidget *createWidget( QWidget *parent );
private:
    void reconnectWidget(SieveIncludeActionWidget *w );
    void updateAddRemoveButton();
};


class SieveIncludeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SieveIncludeWidget(QWidget *parent = 0);
    ~SieveIncludeWidget();
};
}

#endif // SIEVEINCLUDEWIDGET_H
