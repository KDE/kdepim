/* -*- mode: C++; c-file-style: "gnu" -*-

  Copyright (c) 2011, 2012 Montel Laurent <montel@kde.org>

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

#include <KDialog>

class KComboBox;
class KUrlRequester;

class QAbstractItemModel;
class QListWidget;
class QListWidgetItem;
class QModelIndex;

namespace MailCommon {
class FolderRequester;
class AccountList;
}

namespace KPIMIdentities {
class IdentityCombo;
}

namespace MailTransport {
class TransportComboBox;
}

class MAILCOMMON_EXPORT FilterActionMissingCollectionDialog : public KDialog
{
    Q_OBJECT
public:
    explicit FilterActionMissingCollectionDialog( const Akonadi::Collection::List &list,
                                                  const QString &filtername = QString(),
                                                  const QString &argStr = QString(),
                                                  QWidget *parent = 0 );
    ~FilterActionMissingCollectionDialog();

    Akonadi::Collection selectedCollection() const;
    static Akonadi::Collection::List potentialCorrectFolders( const QString &path,
                                                              bool &exactPath );

private Q_SLOTS:
    void slotCurrentItemChanged();
    void slotFolderChanged( const Akonadi::Collection &col );
    void slotDoubleItemClicked( QListWidgetItem *item );

private:
    static void getPotentialFolders( const QAbstractItemModel *model,
                                     const QModelIndex &parentIndex,
                                     const QString &realPath,
                                     Akonadi::Collection::List &list );
    enum collectionEnum {
        IdentifyCollection = Qt::UserRole + 1
    };

private:
    void writeConfig();
    void readConfig();
    MailCommon::FolderRequester *mFolderRequester;
    QListWidget *mListwidget;
};

class FilterActionMissingIdentityDialog : public KDialog
{
    Q_OBJECT
public:
    explicit FilterActionMissingIdentityDialog( const QString &filtername,
                                                QWidget *parent = 0 );
    ~FilterActionMissingIdentityDialog();
    int selectedIdentity() const;

private:
    void writeConfig();
    void readConfig();
    KPIMIdentities::IdentityCombo *mComboBoxIdentity;
};

class FilterActionMissingTransportDialog : public KDialog
{
    Q_OBJECT
public:
    explicit FilterActionMissingTransportDialog( const QString &filtername,
                                                 QWidget *parent = 0 );
    ~FilterActionMissingTransportDialog();
    int selectedTransport() const;

private:
    void writeConfig();
    void readConfig();
    MailTransport::TransportComboBox *mComboBoxTransport;
};

class FilterActionMissingTemplateDialog : public KDialog
{
    Q_OBJECT
public:
    explicit FilterActionMissingTemplateDialog( const QStringList &templateList,
                                                const QString &filtername,
                                                QWidget *parent = 0 );
    ~FilterActionMissingTemplateDialog();
    QString selectedTemplate() const;

private:
    void readConfig();
    void writeConfig();
    KComboBox *mComboBoxTemplate;
};

class FilterActionMissingAccountDialog : public KDialog
{
    Q_OBJECT
public:
    explicit FilterActionMissingAccountDialog( const QStringList &lstAccount,
                                               const QString &filtername,
                                               QWidget *parent = 0 );
    ~FilterActionMissingAccountDialog();
    QStringList selectedAccount() const;
    static bool allAccountExist( const QStringList & lst );

private:
    void readConfig();
    void writeConfig();
    MailCommon::AccountList *mAccountList;
};

class FilterActionMissingTagDialog : public KDialog
{
    Q_OBJECT
public:
    explicit FilterActionMissingTagDialog(const QMap<QUrl, QString> &templateList,
                                          const QString &filtername,
                                          const QString &argsStr,
                                          QWidget *parent = 0 );
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

class FilterActionMissingSoundUrlDialog : public KDialog
{
    Q_OBJECT
public:
    explicit FilterActionMissingSoundUrlDialog( const QString &filtername,
                                                const QString &argStr,
                                                QWidget *parent = 0 );
    ~FilterActionMissingSoundUrlDialog();
    QString soundUrl() const;

private:
    void readConfig();
    void writeConfig();
    KUrlRequester *mUrlWidget;
};

#endif /* FILTERACTIONMISSINGARGUMENTDIALOG_H */

