#ifndef KAB3_VIEWCONTAINER_H
#define KAB3_VIEWCONTAINER_H

#include <qlist.h>

#include "detailsviewcontainer_base.h"
#include "look_basic.h"

class ViewContainer : public DetailsViewContainerBase
{
    Q_OBJECT
public:
    ViewContainer(QWidget *parent=0, const char* name=0);
    /** Return the look currently selected. If there is none, it
        returns zero. Do not use this pointer to store a reference
        to a look, the user might select another one (e.g., create
        a new object) at any time. */
    KABBasicLook *look();
    /** Return the contact currently displayed. */
    KABC::Addressee addressee();
public slots:
    /** Set the contact currently displayed. */
    void setAddressee(const KABC::Addressee& addressee);
    /** Set read-write state. */
    void setReadonly(bool state);
signals:
    /** The contact has been changed. */
    void addresseeChanged();
    /** The user acticated the email address displayed. This may happen
        by, for example, clicking on the displayed mailto-URL. */
    void sendEmail(const QString& address);
    /** The user activated one of the displayed HTTP URLs. For example
        by clicking on the displayed homepage address. */
    void browse(const QString& url);

protected:
    /** A style has been selected. Overloaded from base class. */
    void slotStyleSelected(int);
    /** The active look. Initially zero. */
    KABBasicLook *m_look;
    /** Register the available looks. */
    void registerLooks();
    /** A list of factories that produce looks. */
    QPtrList<KABLookFactory> m_lookFactories;
};


#endif
