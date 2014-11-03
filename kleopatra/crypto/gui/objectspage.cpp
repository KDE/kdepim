/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/gui/objectspage.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2007 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include <config-kleopatra.h>

#include "objectspage.h"

#include <utils/filedialog.h>

#include <QIcon>
#include <KLocalizedString>

#include <QFileInfo>
#include <QListWidget>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStringList>
#include <QVBoxLayout>

#include <cassert>

using namespace Kleo;
using namespace Kleo::Crypto::Gui;

class ObjectsPage::Private
{
    friend class ::Kleo::Crypto::Gui::ObjectsPage;
    ObjectsPage *const q;
public:
    explicit Private(ObjectsPage *qq);
    ~Private();
    void add();
    void addFile(const QFileInfo &i);
    void remove();
    void listSelectionChanged();
    enum Role {
        AbsoluteFilePathRole = Qt::UserRole
    };

private:
    QListWidget *fileListWidget;
    QPushButton *removeButton;
};

ObjectsPage::Private::Private(ObjectsPage *qq)
    : q(qq)
{
    q->setTitle(i18n("<b>Objects</b>"));
    QVBoxLayout *const top = new QVBoxLayout(q);
    fileListWidget = new QListWidget;
    fileListWidget->setSelectionMode(QAbstractItemView::MultiSelection);
    connect(fileListWidget, SIGNAL(itemSelectionChanged()), q, SLOT(listSelectionChanged()));
    top->addWidget(fileListWidget);
    QWidget *const buttonWidget = new QWidget;
    QHBoxLayout *const buttonLayout = new QHBoxLayout(buttonWidget);
    removeButton = new QPushButton;
    removeButton->setText(i18n("Remove Selected"));
    connect(removeButton, SIGNAL(clicked()), q, SLOT(remove()));
    buttonLayout->addWidget(removeButton);
    buttonLayout->addStretch();
    top->addWidget(buttonWidget);
    listSelectionChanged();
}

ObjectsPage::Private::~Private() {}

void ObjectsPage::Private::add()
{
    const QString fname = FileDialog::getOpenFileName(q, i18n("Select File"), QLatin1String("enc"));
    if (fname.isEmpty()) {
        return;
    }
    addFile(QFileInfo(fname));
    emit q->completeChanged();
}

void ObjectsPage::Private::remove()
{
    const QList<QListWidgetItem *> selected = fileListWidget->selectedItems();
    assert(!selected.isEmpty());
    Q_FOREACH (QListWidgetItem *const i, selected) {
        delete i;
    }
    emit q->completeChanged();
}

void ObjectsPage::Private::listSelectionChanged()
{
    removeButton->setEnabled(!fileListWidget->selectedItems().isEmpty());
}

ObjectsPage::ObjectsPage(QWidget *parent, Qt::WindowFlags f)
    : WizardPage(parent, f), d(new Private(this))
{

}

ObjectsPage::~ObjectsPage() {}

void ObjectsPage::setFiles(const QStringList &list)
{
    d->fileListWidget->clear();
    Q_FOREACH (const QString &i, list) {
        d->addFile(QFileInfo(i));
    }
    emit completeChanged();
}

void ObjectsPage::Private::addFile(const QFileInfo &info)
{
    QListWidgetItem *const item = new QListWidgetItem;
    if (info.isDir()) {
        item->setIcon(QIcon::fromTheme(QLatin1String("folder")));
    }
    item->setText(info.fileName());
    item->setData(AbsoluteFilePathRole, info.absoluteFilePath());
    fileListWidget->addItem(item);
}

QStringList ObjectsPage::files() const
{
    QStringList list;
    for (int i = 0; i < d->fileListWidget->count(); ++i) {
        const QListWidgetItem *const item = d->fileListWidget->item(i);
        list.push_back(item->data(Private::AbsoluteFilePathRole).toString());
    }
    return list;
}

bool ObjectsPage::isComplete() const
{
    return d->fileListWidget->count() > 0;
}

#include "moc_objectspage.cpp"

