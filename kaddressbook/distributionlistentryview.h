#ifndef KAB_DISTRIBUTIONLISTENTRYVIEW_H
#define KAB_DISTRIBUTIONLISTENTRYVIEW_H

#include <kabc/distributionlist.h>

#include <QMap>
#include <QString>
#include <QWidget>

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

public slots:
    void clear();

signals:
    void distributionListClicked( const QString& );

private slots:
    void emailButtonClicked( int id );

private:
    QMap<int, QString> m_idToEmail;
    KAB::Core* m_core;
    KABC::DistributionList *m_list;
    KABC::DistributionList::Entry m_entry;
    QGridLayout* m_radioLayout;
    QBoxLayout* m_mainLayout;
    QWidget* m_emailGroup;
    QLabel* m_addresseeLabel;
    KUrlLabel* m_distListLabel;
    QLabel* m_imageLabel;
    QLabel* m_resourceLabel;
    QMap<int, QString> m_indexToIdentifier; 
};

}

#endif // KAB_DISTRIBUTIONLISTENTRYVIEW_H
