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

#ifndef COMBINEDINCIDENCEEDITOR_H
#define COMBINEDINCIDENCEEDITOR_H

#include "incidenceeditor-ng.h"

#include <KMessageWidget>
#include <AkonadiCore/Item>

namespace IncidenceEditorNG
{

/**
 * The CombinedIncidenceEditor combines optional widgets with zero or more
 * IncidenceEditors. The CombinedIncidenceEditor keeps track of the dirty state
 * of the IncidenceEditors that where combined.
 */
class CombinedIncidenceEditor : public IncidenceEditor
{
    Q_OBJECT
public:
    explicit CombinedIncidenceEditor(QWidget *parent = Q_NULLPTR);
    /**
     * Deletes this editor as well as all editors which are combined into this
     * one.
     */
    ~CombinedIncidenceEditor();

    void combine(IncidenceEditor *other);

    /**
     * Returns whether or not the current values in the editor differ from the
     * initial values or if one of the combined editors is dirty.
     */
    bool isDirty() const Q_DECL_OVERRIDE;
    bool isValid() const Q_DECL_OVERRIDE;

    /**
     * Loads all data from @param inicidence into the combined editors. Note, if
     * you reimplement the load method in a subclass, make sure to call this
     * implementation too.
     */
    void load(const KCalCore::Incidence::Ptr &incidence) Q_DECL_OVERRIDE;
    void load(const Akonadi::Item &item) Q_DECL_OVERRIDE;
    void save(const KCalCore::Incidence::Ptr &incidence) Q_DECL_OVERRIDE;
    void save(Akonadi::Item &item) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void handleDirtyStatusChange(bool isDirty);

Q_SIGNALS:
    void showMessage(const QString &reason, KMessageWidget::MessageType) const;

private:
    QVector<IncidenceEditor *> mCombinedEditors;
    int mDirtyEditorCount;
};

}

#endif // COMBINEDINCIDENCEEDITOR_H
