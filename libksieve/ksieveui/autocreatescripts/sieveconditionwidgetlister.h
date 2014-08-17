/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include <libkdepim/widgets/kwidgetlister.h>

class QPushButton;
class QGridLayout;
class QDomElement;

namespace PimCommon {
class MinimumComboBox;
}

namespace KSieveUi {
class SieveCondition;
class SieveHelpButton;
class SieveConditionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SieveConditionWidget(QWidget *parent);
    ~SieveConditionWidget();

    void updateAddRemoveButton( bool addButtonEnabled, bool removeButtonEnabled );
    void generatedScript(QString &script, QStringList &requires);
    void setCondition(const QString &conditionName, const QDomElement &element, bool notCondition, QString &error);

private Q_SLOTS:
    void slotAddWidget();
    void slotRemoveWidget();
    void slotConditionChanged(int index);
    void slotHelp();

Q_SIGNALS:
    void addWidget(QWidget *w);
    void removeWidget(QWidget *w);
    void valueChanged();

private:
    void initWidget();
    void setFilterCondition( QWidget *widget );
    void reset();
    QList<KSieveUi::SieveCondition*> mConditionList;
    QPushButton *mAdd;
    QPushButton *mRemove;
    PimCommon::MinimumComboBox *mComboBox;
    QGridLayout *mLayout;
    SieveHelpButton *mHelpButton;
};

class SieveConditionWidgetLister : public KPIM::KWidgetLister
{
    Q_OBJECT
public:
    explicit SieveConditionWidgetLister(QWidget *parent = 0);
    ~SieveConditionWidgetLister();

    void generatedScript(QString &script, int &numberOfCondition, QStringList &requires);
    int conditionNumber() const;
    void loadScript(const QDomElement &element, bool uniqTest, bool notCondition, QString &error);

Q_SIGNALS:
    void valueChanged();

public Q_SLOTS:
    void slotAddWidget( QWidget *w );
    void slotRemoveWidget( QWidget *w );

protected:
    void clearWidget( QWidget *aWidget );
    QWidget *createWidget( QWidget *parent );

private:
    void loadTest(const QDomElement &e, bool notCondition, QString &error);
    void reconnectWidget(SieveConditionWidget *w );
    void updateAddRemoveButton();
};
}

#endif // SIEVECONDITIONWIDGETLISTER_H
