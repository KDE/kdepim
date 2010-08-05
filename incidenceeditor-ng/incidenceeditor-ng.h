/*
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
    Copyright (C) 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef INCIDENCEEDITOR_NG_H
#define INCIDENCEEDITOR_NG_H

#include <QtGui/QWidget>

#include <kcalcore/incidence.h>

#include "incidenceeditors-ng_export.h"

namespace IncidenceEditorsNG {

/**
 * KCal Incidences are complicated objects. The user interfaces to create/modify
 * are therefore complex too. The IncedenceEditor class is a divide and conquer
 * approach to this complexity. An IncidenceEditor is an editor for a specific
 * part(s) of an Incidence.
 */
class INCIDENCEEDITORS_NG_EXPORT IncidenceEditor : public QObject
{
  Q_OBJECT
  public:
    virtual ~IncidenceEditor();

    /**
     * Load the values of @param incidence into the editor widgets. The passed
     * incidence is kept for comparing with the current values of the editor.
     */
    virtual void load( const KCalCore::Incidence::ConstPtr &incidence ) = 0;

    /**
     * Store the current values of the editor into @param incidince.
     */
    virtual void save( const KCalCore::Incidence::Ptr &incidence ) = 0;

    /**
     * Returns whether or not the current values in the editor differ from the
     * initial values.
     */
    virtual bool isDirty() const = 0;

    /**
     * Returns whether or not the content of this editor is valid. The default
     * implementation returns always true. The method is not const so that
     * re-implementing editors can change the focus of their widgets.
     */
    virtual bool isValid();

    /**
     * Returns a string representation of the Inicdince that is currently loaded.
     */
    QString type();

    /** Convenience method to get a pointer for a specific const Incidence Type. */
    template <typename IncidenceT>
    QSharedPointer<const IncidenceT> incidence() const
    {
      return mLoadedIncidence.dynamicCast<const IncidenceT>();
    }

  signals:
    /**
     * Signals whether the dirty status of this editor has changed. The new dirty
     * status is passed as argument.
     */
    void dirtyStatusChanged( bool isDirty );

  public slots:
    /**
     * Checks if the dirty status has changed until last check and emits the
     * dirtyStatusChanged signal if needed.
     */
    void checkDirtyStatus();

  protected:
    /** Only subclasses can instantiate IncidenceEditors */
    IncidenceEditor( QObject *parent = 0 );

    template <typename IncidenceT>
    QSharedPointer<IncidenceT> incidence( KCalCore::Incidence::Ptr inc )
    {
      return inc.dynamicCast<IncidenceT>();
    }

  protected:
    KCalCore::Incidence::ConstPtr mLoadedIncidence;
    bool mWasDirty;
};

} // IncidenceEditorsNG

#endif // INCIDENCEEDITOR_H
