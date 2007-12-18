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
