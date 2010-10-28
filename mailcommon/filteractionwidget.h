/*
    Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
    Copyright (c) 2010 Andras Mantia <andras@kdab.com>
    Author: Marc Mutz <Marc@Mutz.com>
    based upon work by Stefan Taferner <taferner@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#ifndef MAILCOMMON_FILTERACTIONWIDGET_H
#define MAILCOMMON_FILTERACTIONWIDGET_H

#include "mailcommon_export.h"

#include <KHBox>
#include <kwidgetlister.h>

class QGridLayout;
class KComboBox;

namespace MailCommon {

  class FilterAction;

/** This widgets allows to edit a single MailCommon::FilterAction (in fact
    any derived class that is registered in
    KMFilterActionDict). It consists of a combo box which allows to
    select the type of actions this widget should act upon.

    You can load a MailCommon::FilterAction into this widget with setAction,
    and retrieve the result of user action with action.
    The widget will copy it's setting into the corresponding
    parameter widget. For that, it internally creates an instance of
    every MailCommon::FilterAction in KMFilterActionDict and asks each
    one to create a parameter widget.

    @short A widget to edit a single MailCommon::FilterAction.
    @author Marc Mutz <Marc@Mutz.com>
    @see MailCommon::FilterAction MailCommon::MailFilter FilterActionWidgetLister

 */
class FilterActionWidget : public KHBox
{
  Q_OBJECT
public:
  /** Constructor. Creates a filter action widget with no type
      selected. */
  explicit FilterActionWidget( QWidget* parent=0, const char* name=0 );

  /** Destructor. Clears mActionList. */
  ~FilterActionWidget();

  /** Set an action. The action's type is determined and the
      corresponding widget it loaded with @p aAction's parameters and
      then raised. If @ aAction is 0, the widget is cleared. */
  void setAction( const MailCommon::FilterAction * aAction );
  /** Retrieve the action. This method is necessary because the type
      of actions can change during editing. Therefore the widget
      always creates a new action object from the data in the combo
      box and returns that. */
  MailCommon::FilterAction *action() const;

private slots:
  void slotFilterTypeChanged( int newIdx );

private:
  /** This list holds an instance of every MailCommon::FilterAction
      subclass. The only reason that these 'slave' actions exist is
      that they are 'forced' to create parameter widgets
      and to clear them on setAction. */
  QList<MailCommon::FilterAction*> mActionList;
  /** The combo box that contains the labels of all KMFilterActions.
      It's @p activated(int) signal is internally
      connected to the @p slotCboAction(int) slot of @p FilterActionWidget. */
  KComboBox      *mComboBox;

  void setFilterAction( QWidget* w=0 );
  QGridLayout *gl;
};

class MAILCOMMON_EXPORT FilterActionWidgetLister : public KPIM::KWidgetLister
{
  Q_OBJECT
public:
  explicit FilterActionWidgetLister( QWidget *parent=0, const char* name=0 );

  virtual ~FilterActionWidgetLister();

  void setActionList( QList<MailCommon::FilterAction*> * aList );

  /** Updates the action list according to the current widget values */
  void updateActionList() { regenerateActionListFromWidgets(); }

public slots:
  void reset();

protected:
  virtual void clearWidget( QWidget *aWidget );
  virtual QWidget* createWidget( QWidget *parent );

private:
  void regenerateActionListFromWidgets();
  QList<MailCommon::FilterAction*> *mActionList;

};


}

#endif // FILTERACTIONWIDGET_H
