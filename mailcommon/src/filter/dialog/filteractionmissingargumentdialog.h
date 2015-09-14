/*

  Copyright (c) 2011-2015 Montel Laurent <montel@kde.org>

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

#ifndef MAILCOMMON_FILTERACTIONMISSINGARGUMENTDIALOG_H
#define MAILCOMMON_FILTERACTIONMISSINGARGUMENTDIALOG_H

#include "mailcommon_export.h"

#include <Collection>

#include <QDialog>

class KComboBox;
class KUrlRequester;

class QAbstractItemModel;
class QListWidget;
class QListWidgetItem;
class QModelIndex;
class QPushButton;

namespace MailCommon
{
class FolderRequester;
class KMFilterAccountList;
}

namespace KIdentityManagement
{
class IdentityCombo;
}

namespace MailTransport
{
class TransportComboBox;
}

class MAILCOMMON_EXPORT FilterActionMissingCollectionDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FilterActionMissingCollectionDialog(const Akonadi::Collection::List &list,
            const QString &filtername = QString(),
            const QString &argStr = QString(),
            QWidget *parent = Q_NULLPTR);
    ~FilterActionMissingCollectionDialog();

    Akonadi::Collection selectedCollection() const;
    static Akonadi::Collection::List potentialCorrectFolders(const QString &path,
            bool &exactPath);

private Q_SLOTS:
    void slotCurrentItemChanged();
    void slotFolderChanged(const Akonadi::Collection &col);
    void slotDoubleItemClicked(QListWidgetItem *item);

private:
    static void getPotentialFolders(const QAbstractItemModel *model,
                                    const QModelIndex &parentIndex,
                                    const QString &realPath,
                                    Akonadi::Collection::List &list);
    enum collectionEnum {
        IdentifyCollection = Qt::UserRole + 1
    };

private:
    void writeConfig();
    void readConfig();
    MailCommon::FolderRequester *mFolderRequester;
    QListWidget *mListwidget;
    QPushButton *mOkButton;
};

class FilterActionMissingIdentityDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FilterActionMissingIdentityDialog(const QString &filtername,
            QWidget *parent = Q_NULLPTR);
    ~FilterActionMissingIdentityDialog();
    int selectedIdentity() const;

private:
    void writeConfig();
    void readConfig();
    KIdentityManagement::IdentityCombo *mComboBoxIdentity;
};

class FilterActionMissingTransportDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FilterActionMissingTransportDialog(const QString &filtername,
            QWidget *parent = Q_NULLPTR);
    ~FilterActionMissingTransportDialog();
    int selectedTransport() const;

private:
    void writeConfig();
    void readConfig();
    MailTransport::TransportComboBox *mComboBoxTransport;
};

class FilterActionMissingTemplateDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FilterActionMissingTemplateDialog(const QStringList &templateList,
            const QString &filtername,
            QWidget *parent = Q_NULLPTR);
    ~FilterActionMissingTemplateDialog();
    QString selectedTemplate() const;

private:
    void readConfig();
    void writeConfig();
    KComboBox *mComboBoxTemplate;
};

class FilterActionMissingAccountDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FilterActionMissingAccountDialog(const QStringList &lstAccount,
            const QString &filtername,
            QWidget *parent = Q_NULLPTR);
    ~FilterActionMissingAccountDialog();
    QStringList selectedAccount() const;
    static bool allAccountExist(const QStringList &lst);

private:
    void readConfig();
    void writeConfig();
    MailCommon::KMFilterAccountList *mAccountList;
};

class FilterActionMissingTagDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FilterActionMissingTagDialog(const QMap<QUrl, QString> &templateList,
                                          const QString &filtername,
                                          const QString &argsStr,
                                          QWidget *parent = Q_NULLPTR);
    ~FilterActionMissingTagDialog();
    QString selectedTag() const;

private Q_SLOTS:
    void slotAddTag();

private:
    void readConfig();
    void writeConfig();

    enum TypeData {
        UrlData = Qt::UserRole + 1
    };
    QListWidget *mTagList;
};

class FilterActionMissingSoundUrlDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FilterActionMissingSoundUrlDialog(const QString &filtername,
            const QString &argStr,
            QWidget *parent = Q_NULLPTR);
    ~FilterActionMissingSoundUrlDialog();
    QString soundUrl() const;

private:
    void readConfig();
    void writeConfig();
    KUrlRequester *mUrlWidget;
};

#endif /* FILTERACTIONMISSINGARGUMENTDIALOG_H */

