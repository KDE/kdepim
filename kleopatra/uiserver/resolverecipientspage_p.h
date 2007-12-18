/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/resolverecipientspage_p.h

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

#ifndef __KLEOPATRA_RESOLVERECIPIENTSPAGE_P_H__
#define __KLEOPATRA_RESOLVERECIPIENTSPAGE_P_H__

#include "resolverecipientspage.h"

#include <QHash>

class QComboBox;
class QLabel;
class QListWidget;
class QListWidgetItem;
class QPushButton;
class QStringList;

namespace Kleo {

    class ResolveRecipientsPage::ListWidget : public QWidget {
        Q_OBJECT
    public:
        explicit ListWidget( QWidget* parent = 0, Qt::WindowFlags flags = 0 );
        ~ListWidget();

        void addEntry( const QString& id, const QString& name );
        void removeEntry( const QString& id );
        QStringList selectedEntries() const;
        void setCertificates( const QString& id, const std::vector<GpgME::Key>& pgpCerts, const std::vector<GpgME::Key>& cmsCerts );
        GpgME::Key selectedCertificate( const QString& id ) const;
        QStringList identifiers() const;
        void setProtocol( GpgME::Protocol prot );
        void showSelectionDialog( const QString& id );
        
        enum Role {
            IdRole = Qt::UserRole
        };

    Q_SIGNALS:
        void selectionChanged();
        void completeChanged();

        
    private:        
        QListWidget* m_listWidget;
        
        QHash<QString,ItemWidget*> widgets;
        QHash<QString,QListWidgetItem*> items;
        GpgME::Protocol m_protocol;
    };
    
    class ResolveRecipientsPage::ItemWidget : public QWidget {
        Q_OBJECT
    public:
        explicit ItemWidget( const QString& id, const QString& name, QWidget* parent = 0, Qt::WindowFlags flags = 0 );
        ~ItemWidget();
        
        QString id() const;
        void setCertificates( const std::vector<GpgME::Key>& pgp,
                              const std::vector<GpgME::Key>& cms );
        GpgME::Key selectedCertificate() const;
        std::vector<GpgME::Key> certificates() const;
        void setProtocol( GpgME::Protocol protocol );

    public Q_SLOTS:
        void showSelectionDialog();

    Q_SIGNALS:
        void changed();
        
    private:
        void addCertificateToComboBox( const GpgME::Key& key );
        void resetCertificates();
        void selectCertificateInComboBox( const GpgME::Key& key );
        void updateVisibility();
        
    private:        
        QString m_id;
        QLabel* m_nameLabel;
        QLabel* m_certLabel;
        QComboBox* m_certCombo;
        QPushButton* m_selectButton;
        GpgME::Protocol m_protocol;
        QHash<GpgME::Protocol, GpgME::Key> m_selectedCertificates;
        std::vector<GpgME::Key> m_pgp, m_cms;
    };
}

#endif // __KLEOPATRA_RESOLVERECIPIENTSPAGE_P_H__
