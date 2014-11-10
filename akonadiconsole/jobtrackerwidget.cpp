/*
    This file is part of Akonadi.

    Copyright (c) 2009 Till Adam <adam@kde.org>

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

#include "jobtrackerwidget.h"
#include <QCheckBox>

#include "jobtrackermodel.h"

#include <AkonadiCore/control.h>

#include <KLocalizedString>
#include <QUrl>

#include <QTreeView>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QMenu>
#include <QPushButton>
#include <QFile>
#include <QFileDialog>

class JobTrackerWidget::Private
{
public:
    JobTrackerModel *model;
};

JobTrackerWidget::JobTrackerWidget(const char *name, QWidget *parent, const QString &checkboxText)
    : QWidget(parent),
      d(new Private)
{
    d->model = new JobTrackerModel(name, this);

    QVBoxLayout *layout = new QVBoxLayout(this);

    QCheckBox *enableCB = new QCheckBox(this);
    enableCB->setText(checkboxText);
    connect(enableCB, SIGNAL(toggled(bool)), d->model, SLOT(setEnabled(bool)));
    layout->addWidget(enableCB);

    QTreeView *tv = new QTreeView(this);
    tv->setModel(d->model);
    tv->expandAll();
    tv->setAlternatingRowColors(true);
    tv->setContextMenuPolicy(Qt::CustomContextMenu);
    // too slow with many jobs:
    // tv->header()->setResizeMode( QHeaderView::ResizeToContents );
    connect(d->model, &JobTrackerModel::modelReset, tv, &QTreeView::expandAll);
    connect(tv, &QTreeView::customContextMenuRequested, this, &JobTrackerWidget::contextMenu);
    layout->addWidget(tv);
    d->model->setEnabled(false);   // since it can be slow, default to off

    QHBoxLayout *layout2 = new QHBoxLayout(this);
    QPushButton *button = new QPushButton(QLatin1String("Save to file..."), this);
    connect(button, SIGNAL(clicked(bool)),
            this, SLOT(slotSaveToFile()));
    layout2->addWidget(button);
    layout2->addStretch(1);
    layout->addLayout(layout2);

    Akonadi::Control::widgetNeedsAkonadi(this);
}

JobTrackerWidget::~JobTrackerWidget()
{
    delete d;
}

void JobTrackerWidget::contextMenu(const QPoint &pos)
{
    QMenu menu;
    menu.addAction(i18n("Clear View"), d->model, SLOT(resetTracker()));
    menu.exec(mapToGlobal(pos));
}

void JobTrackerWidget::slotSaveToFile()
{
    const QString fileName = QFileDialog::getSaveFileName(0, QString(), QString(), QString());
    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return;
    }

    file.write("Job ID\t\tCreated\t\tWait Time\tJob Duration\tJob Type\t\tState\tInfo\n");

    writeRows(QModelIndex(), file, 0);

    file.close();
}

void JobTrackerWidget::writeRows(const QModelIndex &parent, QFile &file, int indentLevel)
{
    for (int row = 0; row < d->model->rowCount(parent); ++row) {
        QByteArray data;
        for (int tabs = 0; tabs < indentLevel; ++tabs) {
            data += '\t';
        }
        const int columnCount = d->model->columnCount(parent);
        for (int column = 0; column < columnCount; ++column) {
            const QModelIndex index = d->model->index(row, column, parent);
            data += index.data().toByteArray();
            if (column < columnCount - 1) {
                data += '\t';
            }
        }
        data += '\n';
        file.write(data);

        const QModelIndex index = d->model->index(row, 0, parent);
        writeRows(index, file, indentLevel + 1);
    }
}
