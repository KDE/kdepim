#ifndef KAB_DISTRIBUTIONLISTENTRYVIEW_H
#define KAB_DISTRIBUTIONLISTENTRYVIEW_H

#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtGui/QWidget>

#include <kabc/distributionlist.h>

class QBoxLayout;
class QComboBox;
class QGridLayout;
class QLabel;

class KUrlLabel;

class ImageButton;

namespace KAB {

class Core;

class DistributionListEntryView : public QWidget
{
    Q_OBJECT

public:
    explicit DistributionListEntryView( KAB::Core* core, QWidget* parent = 0 );
    void setEntry( KABC::DistributionList *list, const KABC::DistributionList::Entry &entry );

public Q_SLOTS:
    void clear();

Q_SIGNALS:
    void distributionListClicked( const QString& );

private Q_SLOTS:
    void emailButtonClicked( int id );

private:
    QMap<int, QString> m_idToEmail;
    KAB::Core* m_core;
    KABC::DistributionList *m_list;
    KABC::DistributionList::Entry m_entry;
    QGridLayout* m_radioLayout;
    QBoxLayout* m_mainLayout;
    QWidget* m_emailGroup;
    QWidget* m_mailSelectorWidget;
    QLabel* m_addresseeLabel;
    KUrlLabel* m_distListLabel;
    QLabel* m_imageLabel;
    QLabel* m_resourceLabel;
    QMap<int, QString> m_indexToIdentifier; 
};

}

#endif // KAB_DISTRIBUTIONLISTENTRYVIEW_H
