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

#ifndef SIEVEGLOBALVARIABLEWIDGET_H
#define SIEVEGLOBALVARIABLEWIDGET_H

#include "sievewidgetpageabstract.h"
#include <libkdepim/widgets/kwidgetlister.h>

class KPushButton;
class KLineEdit;
class QGridLayout;
class QCheckBox;
class QDomElement;

namespace KSieveUi {
class SieveGlobalVariableWidget;

class SieveGlobalVariableActionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SieveGlobalVariableActionWidget(QWidget *parent = 0);
    ~SieveGlobalVariableActionWidget();

    void generatedScript(QString &script);
    void updateAddRemoveButton( bool addButtonEnabled, bool removeButtonEnabled );

private Q_SLOTS:
    void slotAddWidget();
    void slotRemoveWidget();

Q_SIGNALS:
    void addWidget(QWidget *w);
    void removeWidget(QWidget *w);

private:
    void initWidget();
    KPushButton *mAdd;
    KPushButton *mRemove;
    QGridLayout *mLayout;
    KLineEdit *mVariableName;
    QCheckBox *mSetValueTo;
    KLineEdit *mVariableValue;
};

class SieveGlobalVariableLister : public KPIM::KWidgetLister
{
    Q_OBJECT
public:
    explicit SieveGlobalVariableLister(QWidget *parent = 0);
    ~SieveGlobalVariableLister();

    void generatedScript(QString &script, QStringList &requires);
public Q_SLOTS:
    void slotAddWidget( QWidget *w );
    void slotRemoveWidget( QWidget *w );

protected:
    void clearWidget( QWidget *aWidget );
    QWidget *createWidget( QWidget *parent );
private:
    void reconnectWidget(SieveGlobalVariableActionWidget *w );
    void updateAddRemoveButton();
};


class SieveGlobalVariableWidget : public SieveWidgetPageAbstract
{
    Q_OBJECT
public:
    explicit SieveGlobalVariableWidget(QWidget *parent = 0);
    ~SieveGlobalVariableWidget();

    void generatedScript(QString &script, QStringList &requires);

    void loadScript(const QDomElement &element);

private Q_SLOTS:
    void slotHelp();

private:
    SieveGlobalVariableLister *mIncludeLister;
};
}


class SieveGlobalVariableWidget
{
public:
    SieveGlobalVariableWidget();
};

#endif // SIEVEGLOBALVARIABLEWIDGET_H
