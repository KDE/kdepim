#ifndef DETAILEDVIEWWRAPPER_H
#define DETAILEDVIEWWRAPPER_H

#include "../viewwrapper.h"

class DetailedViewWrapper : public ViewWrapper
{
public:
    DetailedViewWrapper();
    virtual ~DetailedViewWrapper();
    QString type() const;
    QString description() const;
    KAddressBookView *createView(KABC::AddressBook *doc,
                                 QWidget *parent,
                                 const char *name);
};

#endif
