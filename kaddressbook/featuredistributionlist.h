#ifndef FEATUREDISTRIBUTIONLIST_H
#define FEATUREDISTRIBUTIONLIST_H

#include <kabc/distributionlist.h>
#include "featuredistributionlistbase.h"

class FeatureDistributionList : public FeatureDistributionListBase
{
    Q_OBJECT
public:
    FeatureDistributionList(KABC::AddressBook*, QWidget *parent=0, const char* name=0);
    virtual ~FeatureDistributionList();
    /** Store changes in resource. */
    virtual void commit();
protected:
    /** Set up the displayed information (list of lists etc). */
    void update();
    /** Enable or disable buttons, adjust list member display to
        selection in mCbListSelect. */
    void updateGUI();
    /** The addressbook. */
    KABC::AddressBook *mDoc;
    /** The list manager. */
    KABC::DistributionListManager *mManager;
    /** Catch the show event. */
    void showEvent(QShowEvent *);
    /** Catch the drop event. */
    void dropEvent(QDropEvent*);
    // overloaded slots from the designer created base class:
    void slotListNew();
    void slotListRename();
    void slotListRemove();
    void slotEntryChangeEmail();
    void slotEntryRemove();
    void slotListSelected(int);
public slots:
    /** The selection in the addressee list changed. */
    void slotAddresseeSelectionChanged();
    /** Notification of drop events by the widgets. */
    void slotDropped(QDropEvent*);
signals:
    void modified();
};

#endif
