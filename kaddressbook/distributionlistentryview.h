#ifndef KAB_DISTRIBUTIONLISTENTRYVIEW_H
#define KAB_DISTRIBUTIONLISTENTRYVIEW_H

#include <libkdepim/distributionlist.h>

#include <tqmap.h>
#include <tqstring.h>
#include <tqwidget.h>

class TQBoxLayout;
class TQButtonGroup;
class TQComboBox;
class TQGridLayout;
class TQLabel;

class KURLLabel;

class ImageButton;

namespace KAB {

class Core;

class DistributionListEntryView : public QWidget
{
    Q_OBJECT

public:
    explicit DistributionListEntryView( KAB::Core* core, TQWidget* parent = 0 );
    void setEntry( const KPIM::DistributionList& list, const KPIM::DistributionList::Entry& entry );

public slots:
    void clear();

signals:
    void distributionListClicked( const TQString& );

private slots:
    void emailButtonClicked( int id );

private:
    TQMap<int, TQString> m_idToEmail;
    KAB::Core* m_core;
    KPIM::DistributionList m_list;
    KPIM::DistributionList::Entry m_entry;
    TQGridLayout* m_radioLayout;
    TQBoxLayout* m_mainLayout;
    TQButtonGroup* m_emailGroup;
    TQLabel* m_addresseeLabel;
    KURLLabel* m_distListLabel;
    TQLabel* m_imageLabel;
    TQLabel* m_resourceLabel;
    TQMap<int, TQString> m_indexToIdentifier; 
};

}

#endif // KAB_DISTRIBUTIONLISTENTRYVIEW_H
