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

#include "incidenceeditors_ng_export.h"

#include <KCalCore/Incidence>

namespace IncidenceEditorNG
{

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
    virtual void load(const KCalCore::Incidence::Ptr &incidence) = 0;

    /**
     * Store the current values of the editor into @param incidince.
     */
    virtual void save(const KCalCore::Incidence::Ptr &incidence) = 0;

    /**
     * Returns whether or not the current values in the editor differ from the
     * initial values.
     */
    virtual bool isDirty() const = 0;

    /**
     * Returns whether or not the content of this editor is valid. The default
     * implementation returns always true.
     */
    virtual bool isValid() const;

    /**
       Returns the last error, which is set in isValid() on error,
       and cleared on success.
    */
    QString lastErrorString() const;

    /**
     * Sets focus on the invalid field.
     */
    virtual void focusInvalidField();

    /**
     * Returns the type of the Incidence that is currently loaded.
     */
    KCalCore::IncidenceBase::IncidenceType type() const;

    /** Convenience method to get a pointer for a specific const Incidence Type. */
    template <typename IncidenceT>
    QSharedPointer<IncidenceT> incidence() const
    {
        return mLoadedIncidence.dynamicCast<IncidenceT>();
    }

    /**
       Re-implement this and print important member values and widget
       enabled/disabled states that could have lead to isDirty() returning
       true when the user didn't do any interaction with the editor.

       This method is called in CombinedIncidenceEditor before crashing
       due to assert( !editor->isDirty() )
    */
    virtual void printDebugInfo() const;

signals:
    /**
     * Signals whether the dirty status of this editor has changed. The new dirty
     * status is passed as argument.
     */
    void dirtyStatusChanged(bool isDirty);

public slots:
    /**
     * Checks if the dirty status has changed until last check and emits the
     * dirtyStatusChanged signal if needed.
     */
    void checkDirtyStatus();

protected:
    /** Only subclasses can instantiate IncidenceEditors */
    IncidenceEditor(QObject *parent = 0);

    template <typename IncidenceT>
    QSharedPointer<IncidenceT> incidence(KCalCore::Incidence::Ptr inc)
    {
        return inc.dynamicCast<IncidenceT>();
    }

protected:
    KCalCore::Incidence::Ptr mLoadedIncidence;
    mutable QString mLastErrorString;
    bool mWasDirty;
    bool mLoadingIncidence;
};

} // IncidenceEditorNG

#endif // INCIDENCEEDITOR_H
