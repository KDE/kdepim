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


#ifndef SIEVEACTIONWIDGETLISTER_H
#define SIEVEACTIONWIDGETLISTER_H

#include <libkdepim/widgets/kwidgetlister.h>

class KPushButton;

class QGridLayout;
class QToolButton;
class QDomElement;

namespace PimCommon {
class MinimumComboBox;
}

namespace KSieveUi {
class SieveAction;
class SieveActionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SieveActionWidget(QWidget *parent);
    ~SieveActionWidget();

    void updateAddRemoveButton( bool addButtonEnabled, bool removeButtonEnabled );
    void generatedScript(QString &script, QStringList &requires, bool onlyActions);
    void setAction(const QString &actionName, const QDomElement &element, const QString &comment);

private Q_SLOTS:
    void slotAddWidget();
    void slotRemoveWidget();
    void slotActionChanged(int index);
    void slotHelp();
    void slotAddComment();

Q_SIGNALS:
    void addWidget(QWidget *w);
    void removeWidget(QWidget *w);

private:
    void setFilterAction( QWidget *widget );
    void initWidget();
    QList<KSieveUi::SieveAction*> mActionList;
    KPushButton *mAdd;
    KPushButton *mRemove;
    PimCommon::MinimumComboBox *mComboBox;
    QGridLayout *mLayout;
    QToolButton *mHelpButton;
    QToolButton *mCommentButton;
};


class SieveActionWidgetLister : public KPIM::KWidgetLister
{
    Q_OBJECT
public:
    explicit SieveActionWidgetLister(QWidget *parent = 0);
    ~SieveActionWidgetLister();

    void generatedScript(QString &script, QStringList &requires, bool onlyActions);
    void loadScript(const QDomElement &element, bool onlyActions);

    int actionNumber() const;
public Q_SLOTS:
    void slotAddWidget( QWidget *w );
    void slotRemoveWidget( QWidget *w );

protected:
    void clearWidget( QWidget *aWidget );
    QWidget *createWidget( QWidget *parent );
private:
    void reconnectWidget(SieveActionWidget *w );
    void updateAddRemoveButton();
};
}

#endif // SIEVEACTIONWIDGETLISTER_H
