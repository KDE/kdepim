/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <frank.osterfeld@kdemail.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

//#include "menuitems.h"
#include "tag.h"
#include "tagaction.h"

#include <kapplication.h>
#include <kdebug.h>
#include <kpopupmenu.h>

#include <tqmap.h>
#include <tqpopupmenu.h>


namespace Akregator {

class TagAction::TagActionPrivate
{
    public:
    Tag tag;
    //TQMap<int, TQPopupMenu*> idToPopup;
    //TQMap<TQPopupMenu*, int> popupToId;
};
 
TagAction::TagAction(const Tag& tag, const TQObject *receiver, const char *slot, TQObject *parent)
//KAction (const TQString &text, const KShortcut &cut, const TQObject *receiver, const char *slot, TQObject *parent, const char *name=0)
       : KToggleAction(tag.name(), KShortcut(), 0, 0, parent), d(new TagActionPrivate)
{
     d->tag = tag;
     connect(this, TQT_SIGNAL(toggled(const Tag&, bool)), receiver, slot);
     connect(this, TQT_SIGNAL(toggled(bool)), this, TQT_SLOT(slotToggled(bool)));
}

TagAction::~TagAction()
{
    delete d;
    d = 0;
}

Tag TagAction::tag() const
{
    return d->tag;
}

/*
void TagAction::unplug(TQWidget* widget)
{
    KToggleAction::unplug(widget);

    TQPopupMenu* popup = ::qt_cast<TQPopupMenu *>(widget);
    if (popup)
    {
        d->idToPopup.remove(d->popupToId[popup]);
        d->popupToId.remove(popup);
    }
}*/

/*
int TagAction::plug(TQWidget* widget, int index)
{
    TQPopupMenu* popup = ::qt_cast<TQPopupMenu *>( widget );
    if (!popup)
    {
        kdWarning() << "Can not plug KToggleAction in " << widget->className() << endl;
        return -1;
    }
    if (kapp && !kapp->authorizeKAction(name()))
        return -1;
    
   TagMenuItem* item = new TagMenuItem(d->tag);
    int id = popup->insertItem(TagMenuItem::checkBoxIconSet(isChecked(), popup->colorGroup()), item, -1, index);
   
    
    popup->connectItem (id, this, TQT_SLOT(slotActivated()));

    d->popupToId[popup] = id;
    d->idToPopup[id] = popup;

    if ( id == -1 )
        return id;
    
    return id;   
}
*/
void TagAction::slotToggled(bool enabled)
{
    emit toggled(d->tag, enabled);
}

} // namespace Akregator

#include "tagaction.moc"
