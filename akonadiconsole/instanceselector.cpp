/*
    This file is part of Akonadi.

    Copyright (c) 2012 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#include "instanceselector.h"
#include "ui_instanceselector.h"

#include <QDebug>
#include <QIcon>
#include <KLocalizedString>
#include <akonadi/private/protocol_p.h>

#include <QApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QStandardItemModel>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

InstanceSelector::InstanceSelector(const QString &remoteHost, QWidget *parent, Qt::WindowFlags flags)
    : QDialog(parent, flags),
      ui(new Ui::InstanceSelector),
      m_remoteHost(remoteHost),
      mWindow(0)
{
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    ui->setupUi(mainWidget);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Close);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &InstanceSelector::slotAccept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &InstanceSelector::slotReject);
    mainLayout->addWidget(buttonBox);
    okButton->setIcon(QIcon::fromTheme("network-connect"));
    okButton->setText(i18n("Connect"));

    const QStringList insts = instances();
    qDebug() << "Found running Akonadi instances:" << insts;
    if (insts.size() <= 1) {
        m_instance = QString::fromUtf8(qgetenv("AKONADI_INSTANCE"));
        if (insts.size() == 1 && m_instance.isEmpty()) {
            m_instance = insts.first();
        }
        slotAccept();
    } else {
        QStandardItemModel *model = new QStandardItemModel(this);
        foreach (const QString &inst, insts) {
            QStandardItem *item = new QStandardItem;
            item->setText(inst.isEmpty() ? i18n("<global>") : inst);
            item->setData(inst, Qt::UserRole);
            model->appendRow(item);
        }
        connect(ui->listView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(accept()));
        ui->listView->setModel(model);
        show();
    }
}

InstanceSelector::~InstanceSelector()
{
    delete mWindow;
}

void InstanceSelector::slotAccept()
{
    if (ui->listView->model()) {   // there was something to select
        const QModelIndexList selection = ui->listView->selectionModel()->selectedRows();
        if (selection.size() != 1) {
            return;
        }
        m_instance = selection.first().data(Qt::UserRole).toString();
    }
    QDialog::accept();

    qputenv("AKONADI_INSTANCE", m_instance.toUtf8());
    MainWindow *mWindow = new MainWindow;
    if (!m_remoteHost.isEmpty()) {
        mWindow->setWindowTitle(i18n("Remote Akonadi Console (%1)", m_remoteHost));
    } else if (!m_instance.isEmpty()) {
        mWindow->setWindowTitle(i18n("Akonadi Console (Instance: %1)", m_instance));
    }
    mWindow->show();
}

void InstanceSelector::slotReject()
{
    QDialog::reject();
    QApplication::quit();
}

QStringList InstanceSelector::instances()
{
    const QStringList allServices = QDBusConnection::sessionBus().interface()->registeredServiceNames();
    QStringList insts;
    foreach (const QString &service, allServices) {
        if (!service.startsWith(QLatin1String(AKONADI_DBUS_CONTROL_SERVICE_LOCK))) {
            continue;
        }
        insts.push_back(service.mid(qstrlen(AKONADI_DBUS_CONTROL_SERVICE_LOCK) + 1));
    }
    return insts;
}
