#ifndef KAB_DISTRIBUTIONLISTENTRYVIEW_H
#define KAB_DISTRIBUTIONLISTENTRYVIEW_H

#include <libkdepim/distributionlist.h>

#include <qmap.h>
#include <qstring.h>
#include <qwidget.h>

class QBoxLayout;
class QButtonGroup;
class QComboBox;
class QGridLayout;
class QLabel;

class KURLLabel;

class ImageButton;

namespace KAB {

class Core;

class DistributionListEntryView : public QWidget
{
    Q_OBJECT

public:
    explicit DistributionListEntryView( KAB::Core* core, QWidget* parent = 0 );
    void setEntry( const KPIM::DistributionList& list, const KPIM::DistributionList::Entry& entry );

public slots:
    void clear();

signals:
    void distributionListClicked( const QString& );

private slots:
    void emailButtonClicked( int id );

private:
    QMap<int, QString> m_idToEmail;
    KAB::Core* m_core;
    KPIM::DistributionList m_list;
    KPIM::DistributionList::Entry m_entry;
    QGridLayout* m_radioLayout;
    QBoxLayout* m_mainLayout;
    QButtonGroup* m_emailGroup;
    QLabel* m_addresseeLabel;
    KURLLabel* m_distListLabel;
    QLabel* m_imageLabel;
    QLabel* m_resourceLabel;
    QMap<int, QString> m_indexToIdentifier; 
};

}

#endif // KAB_DISTRIBUTIONLISTENTRYVIEW_H
