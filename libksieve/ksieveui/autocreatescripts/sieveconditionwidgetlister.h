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


#ifndef SIEVECONDITIONWIDGETLISTER_H
#define SIEVECONDITIONWIDGETLISTER_H

#include <libkdepim/kwidgetlister.h>

class KPushButton;

namespace PimCommon {
class MinimumComboBox;
}

namespace KSieveUi {

class SieveConditionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SieveConditionWidget(QWidget *);
    ~SieveConditionWidget();
private Q_SLOTS:
    void slotAddWidget();
    void slotRemoveWidget();
    void slotActionChanged(const QString &action);
Q_SIGNALS:
    void addWidget(QWidget *w);
    void removeWidget(QWidget *w);
private:
    void initWidget();
    void reset();
    KPushButton *mAdd;
    KPushButton *mRemove;
    PimCommon::MinimumComboBox *mComboBox;
};

class SieveConditionWidgetLister : public KPIM::KWidgetLister
{
    Q_OBJECT
public:
    explicit SieveConditionWidgetLister(QWidget *parent = 0);
    ~SieveConditionWidgetLister();

    void generatedScript(QString &script);
public Q_SLOTS:
    void slotAddWidget( QWidget *w );
    void slotRemoveWidget( QWidget *w );
protected:
    void clearWidget( QWidget *aWidget );
    QWidget *createWidget( QWidget *parent );
private:
    void reconnectWidget(SieveConditionWidget *w );
    void updateAddRemoveButton();
};
}

#endif // SIEVECONDITIONWIDGETLISTER_H
