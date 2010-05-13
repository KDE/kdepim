/*
    Copyright (C) 2010  Bertjan Broeksema b.broeksema@home.nl

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

#ifndef COMBINEDINCIDENCEEDITOR_H
#define COMBINEDINCIDENCEEDITOR_H

#include <incidenceeditor-ng/incidenceeditor.h>

/**
 * The CombinedIncidenceEditor combines optional widgets with zero or more
 * IncidenceEditors. The CombinedIncidenceEditor keeps track of the dirty state
 * of the IncidenceEditors that where combined.
 */
class CombinedIncidenceEditor : public IncidenceEditor
{
  Q_OBJECT;
  public:
    /**
     * Returns wether or not the current values in the editor differ from the
     * initial values or if one of the combined editors is dirty.
     */
    virtual bool isDirty() const;
    
  protected:
    CombinedIncidenceEditor( QWidget *parent = 0);
    
    void combine( IncidenceEditor *other );

  private slots:
    void handleDirtyStatusChange( bool isDirty );
    
  private:
    int mDirtyEditorCount;
};

#endif // COMBINEDINCIDENCEEDITOR_H
