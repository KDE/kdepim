/* -*- mode: c++; c-basic-offset:4 -*-
    dialogs/signcertificatedialog_p.h

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

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

#ifndef __KLEOPATRA_DIALOGS_CERTIFYCERTIFICATEDIALOG_P_H
#define __KLEOPATRA_DIALOGS_CERTIFYCERTIFICATEDIALOG_P_H

#include "ui_selectchecklevelwidget.h"
#include "ui_certificationoptionswidget.h"

#include <kleo/signkeyjob.h>

#include <gpgme++/key.h>

#include <QStandardItemModel>
#include <QWizardPage>

class QListView;
class QLabel;
class QCheckBox;

namespace Kleo
{
namespace Dialogs
{
namespace CertifyCertificateDialogPrivate
{
class UserIDModel : public QStandardItemModel
{
    Q_OBJECT
public:
    enum Role {
        UserIDIndex = Qt::UserRole
    };
    explicit UserIDModel(QObject *parent = 0) : QStandardItemModel(parent) {}
    GpgME::Key certificateToCertify() const
    {
        return m_key;
    }
    void setCertificateToCertify(const GpgME::Key &key);
    void setCheckedUserIDs(const std::vector<unsigned int> &uids);
    std::vector<unsigned int> checkedUserIDs() const;

private:
    GpgME::Key m_key;
};

class SecretKeysModel : public QStandardItemModel
{
    Q_OBJECT
public:
    enum Role {
        IndexRole = Qt::UserRole
    };
    explicit SecretKeysModel(QObject *parent = 0) : QStandardItemModel(parent) {}
    void setSecretKeys(const std::vector<GpgME::Key> &keys);
    std::vector<GpgME::Key> secretKeys() const;
    GpgME::Key keyFromItem(const QStandardItem *item) const;
    GpgME::Key keyFromIndex(const QModelIndex &index) const;

private:
    std::vector<GpgME::Key> m_secretKeys;
};

class SelectUserIDsPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit SelectUserIDsPage(QWidget *parent = 0);
    /* reimp */ bool isComplete() const;

    void setSelectedUserIDs(const std::vector<unsigned int> &indexes);
    std::vector<unsigned int> selectedUserIDs() const;
    void setCertificateToCertify(const GpgME::Key &ids);
    GpgME::Key certificateToCertify() const
    {
        return m_userIDModel.certificateToCertify();
    }

private:
    QListView *m_listView;
    QLabel *m_label;
    QCheckBox *m_checkbox;
    UserIDModel m_userIDModel;
};

class SelectCheckLevelPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit SelectCheckLevelPage(QWidget *parent = 0);
    unsigned int checkLevel() const;
private:
    Ui::SelectCheckLevelWidget m_ui;
};

class OptionsPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit OptionsPage(QWidget *parent = 0);

    bool exportableCertificationSelected() const;
    void setCertificatesWithSecretKeys(const std::vector<GpgME::Key> &keys);
    GpgME::Key selectedSecretKey() const;
    bool sendToServer() const;

    /* reimp */ bool validatePage();
    /* reimp */ bool isComplete() const;

Q_SIGNALS:
    void nextClicked();

private:
    Ui::CertificationOptionsWidget m_ui;
    SecretKeysModel m_model;
};

class SummaryPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit SummaryPage(QWidget *parent = 0);
    /* reimp */ bool isComplete() const;
    void setComplete(bool complete);

    void setResult(const GpgME::Error &err);

    struct Summary {
        std::vector<unsigned int> selectedUserIDs;
        unsigned int checkLevel;
        GpgME::Key certificateToCertify;
        GpgME::Key secretKey;
        bool exportable;
        bool sendToServer;
    };

    void setSummary(const Summary &summary);

private:
    bool m_complete;
    QLabel *m_userIDsLabel;
    QLabel *m_secretKeyLabel;
    QLabel *m_checkLevelLabel;
    QLabel *m_resultLabel;
};
}
}
}

#endif

