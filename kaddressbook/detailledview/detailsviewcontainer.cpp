#include "detailsviewcontainer.h"
#include "detailsviewcontainer.moc"

#include <qcombobox.h>
#include <qframe.h>
#include <qlayout.h>
#include <qlabel.h>

#include <kdebug.h>

#include "look_basic.h"
#include "look_details.h"

ViewContainer::ViewContainer(QWidget *parent, const char* name )
    : DetailsViewContainerBase(parent, name),
      m_look(0)
{
    registerLooks();
    // <HACK>: delete temporary widget (designer does not want to add a
    // layout without it):
    delete labelHack; labelHack=0;
    // </HACK>
}


KABBasicLook *ViewContainer::look()
{
    return m_look;
}

void ViewContainer::registerLooks()
{
    m_lookFactories.append(new KABDetailedViewFactory(frameDetails));
    cbStyle->clear();
    for(unsigned int i=0; i<m_lookFactories.count(); ++i)
    {
        cbStyle->insertItem(m_lookFactories.at(i)->description());
    }
    // selected first look:
    if(!m_lookFactories.isEmpty())
    {
        slotStyleSelected(0);
    }
}

void ViewContainer::slotStyleSelected(int index)
{
     if(index>=0 && index<cbStyle->count())
        {
            if(m_look!=0)
            {
                // WORK_TO_DO: save changes
                delete m_look;
                m_look=0;
            }
            KABLookFactory *factory=m_lookFactories.at(index);
            kdDebug() << "ViewContainer::slotStyleSelected: "
                      << "creating look "
                      << factory->description() << endl;
            m_look=factory->create();
            // <HACK>:
            frameDetails->layout()->add(m_look);
            // </HACK>
        }
     // WORK_TO_DO: set current entry
}

