/*
  This file is part of KAddressBook.
  Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "templateselectiondialog.h"

#include <KConfig>
#include <KLocalizedString>
#include <KMessageBox>
#include <QStandardPaths>
#include <QVBoxLayout>
#include <QIcon>
#include <QDir>

#include <QPushButton>
#include <QAbstractTableModel>
#include <QFile>
#include <QFileInfo>
#include <QLabel>
#include <QListView>
#include <QMouseEvent>
#include <QStyledItemDelegate>
#include <KConfigGroup>
#include <QDialogButtonBox>

typedef struct {
    QString displayName;
    QString fileName;
    bool isDeletable;
} TemplateInfo;

class TemplatesModel : public QAbstractTableModel
{
public:
    TemplatesModel(QObject *parent = Q_NULLPTR)
        : QAbstractTableModel(parent)
    {
        update();
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE
    {
        if (!parent.isValid()) {
            return mTemplates.count();
        } else {
            return 0;
        }
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE
    {
        if (!parent.isValid()) {
            return 2;
        } else {
            return 0;
        }
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE
    {
        if (!index.isValid() || index.row() >= mTemplates.count() || index.column() >= 2) {
            return QVariant();
        }

        if (role == Qt::DisplayRole) {
            if (index.column() == 0) {
                return mTemplates[ index.row() ].displayName;
            } else {
                return mTemplates[ index.row() ].fileName;
            }
        }

        if (role == Qt::UserRole) {
            return mTemplates[ index.row() ].isDeletable;
        }

        return QVariant();
    }

    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) Q_DECL_OVERRIDE {
        if (parent.isValid() || row < 0 || row >= mTemplates.count())
        {
            return false;
        }

        beginRemoveRows(parent, row, row + count - 1);
        for (int i = 0; i < count; ++i)
        {
            if (!QFile::remove(mTemplates[ row ].fileName)) {
                return false;
            }
            mTemplates.removeAt(row);
        }

        endRemoveRows();
        return true;
    }

    void update()
    {
        beginResetModel();
        mTemplates.clear();
        const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("/kaddressbook/csv-templates/"), QStandardPaths::LocateDirectory);
        Q_FOREACH (const QString &dir, dirs) {
            const QStringList fileNames = QDir(dir).entryList(QStringList() <<  QStringLiteral("*.desktop"));
            Q_FOREACH (const QString &file, fileNames) {
                const QString fileName = dir + QLatin1Char('/') + file;

                KConfig config(fileName, KConfig::SimpleConfig);

                if (!config.hasGroup("csv column map")) {
                    continue;
                }

                KConfigGroup group(&config, "Misc");
                TemplateInfo info;
                info.displayName = group.readEntry("Name");
                info.fileName = fileName;

                const QFileInfo fileInfo(info.fileName);
                info.isDeletable = QFileInfo(fileInfo.absolutePath()).isWritable();

                mTemplates.append(info);
            }
        }
        endResetModel();
    }

    bool templatesAvailable() const
    {
        return !mTemplates.isEmpty();
    }

private:
    QList<TemplateInfo> mTemplates;
};

class TemplateSelectionDelegate : public QStyledItemDelegate
{
public:
    explicit TemplateSelectionDelegate(QObject *parent = Q_NULLPTR)
        : QStyledItemDelegate(parent), mIcon(QIcon::fromTheme(QStringLiteral("list-remove")))
    {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const Q_DECL_OVERRIDE
    {
        QStyledItemDelegate::paint(painter, option, index);

        if (index.data(Qt::UserRole).toBool()) {
            mIcon.paint(painter, option.rect, Qt::AlignRight);
        }
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE
    {
        QSize hint = QStyledItemDelegate::sizeHint(option, index);

        if (index.data(Qt::UserRole).toBool()) {
            hint.setWidth(hint.width() + 16);
        }

        return hint;
    }

    bool editorEvent(QEvent *event, QAbstractItemModel *model,
                     const QStyleOptionViewItem &option, const QModelIndex &index) Q_DECL_OVERRIDE {
        if (event->type() == QEvent::MouseButtonRelease && index.data(Qt::UserRole).toBool())
        {
            const QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            QRect buttonRect = option.rect;
            buttonRect.setLeft(buttonRect.right() - 16);

            if (buttonRect.contains(mouseEvent->pos())) {
                const QString templateName = index.data(Qt::DisplayRole).toString();
                if (KMessageBox::questionYesNo(
                    Q_NULLPTR,
                    i18nc("@label", "Do you really want to delete template '%1'?",
                templateName)) == KMessageBox::Yes) {
                    model->removeRows(index.row(), 1);
                    return true;
                }
            }
        }

        return QStyledItemDelegate::editorEvent(event, model, option, index);
    }

private:
    QIcon mIcon;
};

TemplateSelectionDialog::TemplateSelectionDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18nc("@title:window", "Template Selection"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mOkButton = buttonBox->button(QDialogButtonBox::Ok);
    mOkButton->setDefault(true);
    mOkButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &TemplateSelectionDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &TemplateSelectionDialog::reject);

    QWidget *wdg = new QWidget(this);
    QVBoxLayout *wdgVBoxLayout = new QVBoxLayout(wdg);
    wdgVBoxLayout->setMargin(0);

    new QLabel(i18nc("@info", "Please select a template, that matches the CSV file:"), wdg);

    mView = new QListView(wdg);
    wdgVBoxLayout->addWidget(mView);

    mView->setModel(new TemplatesModel(this));
    mView->setItemDelegate(new TemplateSelectionDelegate(this));

    mOkButton->setEnabled(false);
    mainLayout->addWidget(wdg);
    mainLayout->addWidget(buttonBox);
    connect(mView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &TemplateSelectionDialog::updateButtons);
}

bool TemplateSelectionDialog::templatesAvailable() const
{
    return static_cast<TemplatesModel *>(mView->model())->templatesAvailable();
}

QString TemplateSelectionDialog::selectedTemplate() const
{
    const QModelIndex rowIndex = mView->currentIndex();
    const QModelIndex index = mView->model()->index(rowIndex.row(), 1);

    return index.data(Qt::DisplayRole).toString();
}

void TemplateSelectionDialog::updateButtons()
{
    mOkButton->setEnabled(mView->currentIndex().isValid());
}

