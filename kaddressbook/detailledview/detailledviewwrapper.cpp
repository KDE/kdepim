#include <klocale.h>
#include <kconfig.h>
#include <kuniqueapplication.h>
#include "detailledviewwrapper.h"
#include "detailledview.h"

DetailedViewWrapper::DetailedViewWrapper()
    : ViewWrapper()
{
}

DetailedViewWrapper::~DetailedViewWrapper()
{
}

QString DetailedViewWrapper::type() const
{
    return i18n("Detailed view");
}

QString DetailedViewWrapper::description() const
{
    return i18n("<qt>This view displays details of a selected contact, "
                "in addition to the compact overview.</qt>");
}

KAddressBookView* DetailedViewWrapper::createView(KABC::AddressBook *doc,
                                                  QWidget *parent,
                                                  const char *name)
{
    KConfig *config;
    KAddressBookDetailedView *view;
    config=kapp->config();
    view=new KAddressBookDetailedView(doc, parent, name);
    view->init(config);
    return view;
}

