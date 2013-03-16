/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include "configureactivitywidget.h"
#include "activitymanager.h"
#include "widgets/configureidentity.h"
#include "widgets/comboboxactivity.h"
#include "widgets/configuremailtransport.h"
#include "widgets/configurecollections.h"
#include "widgets/activitywarning.h"

#include <KTabWidget>
#include <KLocale>

#include <QHBoxLayout>
#include <QCheckBox>
#include <QLabel>

namespace PimActivity {

class ConfigureActivityWidgetPrivate {
public:
    ConfigureActivityWidgetPrivate(ActivityManager *activityManager, ConfigureActivityWidget * qq)
        : q(qq),
          activateActivity( 0 ),
          tabWidget( 0 ),
          manager( activityManager ),
          identity( 0 ),
          mailTransport( 0 )
    {
        QVBoxLayout * lay = new QVBoxLayout;
        activateActivity = new QCheckBox(i18n("Enable Support Activity"));

        lay->addWidget(activateActivity);

        lay->addWidget(new ActivityWarning(manager));

        QHBoxLayout *verticalLayout = new QHBoxLayout;
        QLabel *lab = new QLabel(i18n("Activities:"));
        verticalLayout->addWidget(lab);

        activities = new ComboBoxActivity(manager, q);
        verticalLayout->addWidget(activities);


        lay->addLayout(verticalLayout);
        tabWidget = new KTabWidget;

        lay->addWidget(tabWidget);
        q->setLayout(lay);
        q->connect(activateActivity, SIGNAL(toggled(bool)), activities, SLOT(setEnabled(bool)));
        q->connect(activateActivity, SIGNAL(toggled(bool)), tabWidget, SLOT(setEnabled(bool)));

        addPages();
    }
    ~ConfigureActivityWidgetPrivate()
    {
    }

    void readConfig()
    {
        identity->readConfig();
        mailTransport->readConfig();
        collections->readConfig();
    }

    void writeConfig()
    {
        identity->writeConfig();
        mailTransport->writeConfig();
        collections->writeConfig();
    }

    void addPages()
    {
        identity = new ConfigureIdentity(q);
        tabWidget->addTab(identity, i18n("Identity"));

        mailTransport = new ConfigureMailtransport;
        tabWidget->addTab(mailTransport, i18n("Transport"));

        collections = new ConfigureCollections;
        tabWidget->addTab(collections, i18n("Collections"));
    }

    ConfigureActivityWidget *q;
    QCheckBox *activateActivity;
    KTabWidget *tabWidget;
    ActivityManager *manager;
    ConfigureIdentity *identity;
    ComboBoxActivity *activities;
    ConfigureMailtransport *mailTransport;
    ConfigureCollections *collections;
};

ConfigureActivityWidget::ConfigureActivityWidget(ActivityManager *manager, QWidget *parent)
    : QWidget(parent), d(new ConfigureActivityWidgetPrivate(manager, this))
{
}

ConfigureActivityWidget::~ConfigureActivityWidget()
{
    delete d;
}

void ConfigureActivityWidget::readConfig()
{
    d->readConfig();
}

void ConfigureActivityWidget::writeConfig()
{
    d->writeConfig();
}

void ConfigureActivityWidget::defaults()
{
    //TODO
}

}


#include "configureactivitywidget.moc"
