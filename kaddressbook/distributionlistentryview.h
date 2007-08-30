#ifndef KAB_DISTRIBUTIONLISTENTRYVIEW_H
#define KAB_DISTRIBUTIONLISTENTRYVIEW_H

#include <libkdepim/distributionlist.h>

#include <qwidget.h>

class QButtonGroup;
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

private:
    void writeBackChanges();

private:
    KAB::Core* m_core;
    KPIM::DistributionList m_list;
    KPIM::DistributionList::Entry m_entry;
    QGridLayout* m_radioLayout;
    QButtonGroup* m_emailGroup;
    QLabel* m_addresseeLabel;
    KURLLabel* m_distListLabel;
    QLabel* m_imageLabel;
};

}

#endif // KAB_DISTRIBUTIONLISTENTRYVIEW_H
