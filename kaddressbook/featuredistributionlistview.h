#ifndef FEATUREDISTRIBUTIONLISTVIEW_H
#define FEATUREDISTRIBUTIONLISTVIEW_H

#include <klistview.h>

class FeatureDistributionListView : public KListView
{
    Q_OBJECT
public:
    FeatureDistributionListView(QWidget *parent, const char* name=0);
protected:
    void dragEnterEvent(QDragEnterEvent* event);
    void dropEvent(QDropEvent *e);
signals:
    void dropped(QDropEvent *e);
};
#endif
