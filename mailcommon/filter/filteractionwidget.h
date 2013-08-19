/*
  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Copyright (c) 2010 Andras Mantia <andras@kdab.com>
  Author: Marc Mutz <mutz@kde.org>
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

#include <libkdepim/widgets/kwidgetlister.h>

#include <KHBox>

namespace MailCommon {

class FilterAction;
class FilterActionWidget;

/**
 * @short A widget to edit a single MailCommon::FilterAction.
 *
 * This widgets allows to edit a single MailCommon::FilterAction (in fact
 * any derived class that is registered in
 * KMFilterActionDict). It consists of a combo box which allows to
 * select the type of actions this widget should act upon.
 *
 * You can load a MailCommon::FilterAction into this widget with setAction,
 * and retrieve the result of user action with action.
 * The widget will copy it's setting into the corresponding
 * parameter widget. For that, it internally creates an instance of
 * every MailCommon::FilterAction in KMFilterActionDict and asks each
 * one to create a parameter widget.
 *
 * @author Marc Mutz <mutz@kde.org>
 * @see MailCommon::FilterAction MailCommon::MailFilter FilterActionWidgetLister
 */
class FilterActionWidget : public KHBox
{
  Q_OBJECT

  public:
    /**
     * Creates a filter action widget with no type selected.
     *
     * @param parent The parent widget.
     */
    explicit FilterActionWidget( QWidget *parent = 0 );

    /**
     * Destroys the filter action widget.
     */
    ~FilterActionWidget();

    /**
     * Sets the filter action.
     *
     * The action's type is determined and the corresponding widget
     * it loaded with @p action's parameters and then raised.
     *
     * If @p action is @c 0, the widget is cleared.
     * @note The widget takes ownership of the passed action.
     */
    void setAction( const MailCommon::FilterAction *action );

    /**
     * Returns the filter action.
     *
     * This method is necessary because the type of actions can
     * change during editing. Therefore the widget always creates a new
     * action object from the data in the combo box and returns that.
     */
    MailCommon::FilterAction *action() const;

    void updateAddRemoveButton( bool addButtonEnabled, bool removeButtonEnabled );

  signals:
    void filterModified();
    void addWidget( QWidget * );
    void removeWidget( QWidget * );

  private:
    //@cond PRIVATE
    class Private;
    Private *const d;

    Q_PRIVATE_SLOT( d, void slotFilterTypeChanged( int ) )
    Q_PRIVATE_SLOT( d, void slotAddWidget() )
    Q_PRIVATE_SLOT( d, void slotRemoveWidget() )
    //@endcond
};

/**
 * @short A container widget for a list of FilterActionWidgets.
 *
 * @author Marc Mutz <mutz@kde.org>
 * @see MailCommon::FilterAction MailCommon::MailFilter FilterActionWidget
 */
class MAILCOMMON_EXPORT FilterActionWidgetLister : public KPIM::KWidgetLister
{
  Q_OBJECT

  public:
    /**
     * Creates a new filter action widget lister.
     *
     * @param parent The parent widget.
     */
    explicit FilterActionWidgetLister( QWidget *parent = 0 );

    /**
     * Destroys the filter action widget lister.
     */
    virtual ~FilterActionWidgetLister();

    /**
     * Sets the @p list of filter actions, the lister will create FilterActionWidgets for.
     */
    void setActionList( QList<MailCommon::FilterAction*> *list );

    /**
     * Updates the action list according to the current action widget values.
     */
    void updateActionList();

    void reconnectWidget( FilterActionWidget *w );

  public Q_SLOTS:
    /**
     * Resets the action widgets.
     */
    void reset();
    void slotAddWidget( QWidget * );
    void slotRemoveWidget( QWidget * );

  signals:
    void filterModified();

  protected:
    /**
     * @copydoc KPIM::KWidgetLister::clearWidget
     */
    virtual void clearWidget( QWidget * );

    /**
     * @copydoc KPIM::KWidgetLister::createWidget
     */
    virtual QWidget *createWidget( QWidget * );

    void updateAddRemoveButton();

  private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};

}

#endif
