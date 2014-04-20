/*
 * Copyright 2014  Sandro Knauß <knauss@kolabsys.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "resourcemanagement.h"
#include "ui_resourcemanagement.h"
#include "resourcemodel.h"

#include <kldap/ldapobject.h>

#include <QStringList>
#include <QLabel>

using namespace IncidenceEditorNG;

ResourceManagement::ResourceManagement()
{
    ui = new Ui::ResourceManagement;
    ui->setupUi(this);

    QStringList attrs;
    attrs << QLatin1String("cn") << QLatin1String("mail") << QLatin1String("givenname") << QLatin1String("sn");

    ResourceModel *model = new ResourceModel(attrs);
    ui->treeResults->setModel(model);

    // This doesn't work till now :( -> that's why i use the clieck signal
    ui->treeResults->setSelectionMode(QAbstractItemView::SingleSelection);
    selectionModel = ui->treeResults->selectionModel();

    connect(ui->resourceSearch, SIGNAL(textChanged(const QString&)),
            SLOT(slotStartSearch(const QString&)));

    connect(ui->treeResults, SIGNAL(clicked(const QModelIndex &)),
            SLOT(slotShowDetails(const QModelIndex &)));

}

void ResourceManagement::slotStartSearch(const QString &text)
{
    ((ResourceModel*)ui->treeResults->model())->startSearch(text);
}

void ResourceManagement::slotShowDetails(const QModelIndex & current)
{
    ResourceItem *item = ((ResourceModel*)current.model())->getItem(current);
    showDetails(item->ldapObject());
}


void ResourceManagement::showDetails(const KLDAP::LdapObject &obj)
{
    // Clean up formDetails
    QLayoutItem *child;
    while ((child = ui->formDetails->takeAt(0)) != 0) {
        delete child->widget();
        delete child;
    }

    // Fill formDetails with data
    foreach(const QString & key, obj.attributes().keys()) {
        QStringList list;
        foreach(const QByteArray & value, obj.attributes().value(key)) {
            list << QString::fromUtf8(value);
        }
        ui->formDetails->addRow(key, new QLabel(list.join("\n")));
    }
}

#include "resourcemanagement.moc"
