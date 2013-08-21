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

#include "sievewidgetpageabstract.h"
#include <libkdepim/widgets/kwidgetlister.h>
#include <KComboBox>

class KPushButton;
class KLineEdit;
class QGridLayout;
class QCheckBox;
class QDomElement;
namespace KSieveUi {

class SieveIncludeLocation : public KComboBox
{
    Q_OBJECT
public:
    explicit SieveIncludeLocation(QWidget *parent = 0);
    ~SieveIncludeLocation();

    QString code() const;
    void setCode(const QString &code, QString &error);

private:
    void initialize();
};

class SieveIncludeActionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SieveIncludeActionWidget(QWidget *parent = 0);
    ~SieveIncludeActionWidget();

    void generatedScript(QString &script);
    void updateAddRemoveButton( bool addButtonEnabled, bool removeButtonEnabled );
    void loadScript(const QDomElement &element, QString &error);
    bool isInitialized() const;

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
    QCheckBox *mOptional;
    QCheckBox *mOnce;
    SieveIncludeLocation *mLocation;
    KLineEdit *mIncludeName;
};

class SieveIncludeWidgetLister : public KPIM::KWidgetLister
{
    Q_OBJECT
public:
    explicit SieveIncludeWidgetLister(QWidget *parent = 0);
    ~SieveIncludeWidgetLister();

    void generatedScript(QString &script, QStringList &requires);
    void loadScript(const QDomElement &element, QString &error);

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


class SieveIncludeWidget : public SieveWidgetPageAbstract
{
    Q_OBJECT
public:
    explicit SieveIncludeWidget(QWidget *parent = 0);
    ~SieveIncludeWidget();

    void generatedScript(QString &script, QStringList &requires);

    void loadScript(const QDomElement &element, QString &error);

private Q_SLOTS:
    void slotHelp();

private:
    SieveIncludeWidgetLister *mIncludeLister;
};
}

#endif // SIEVEINCLUDEWIDGET_H
