#include <qdragobject.h>

#include <kdebug.h>

#include "featuredistributionlistview.h"

FeatureDistributionListView::FeatureDistributionListView(QWidget *parent,
                                                         const char* name)
    : KListView(parent, name)
{
    setAcceptDrops(true);
}

void FeatureDistributionListView::dragEnterEvent(QDragEnterEvent* event)
{
    bool canDecode=QTextDrag::canDecode(event);
    kdDebug() << "FeatureDistributionListView::dragEnterEvent: "
              << (canDecode ? "can" : "cannot")
              << " decode this." << endl;
    event->acceptAction(canDecode);
}

void FeatureDistributionListView::dropEvent(QDropEvent *e)
{
    kdDebug() << "FeatureDistributionListView::dropEvent: got it." << endl;
    emit(dropped(e));
}

#include "featuredistributionlistview.moc"
