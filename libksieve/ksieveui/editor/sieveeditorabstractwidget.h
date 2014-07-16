/* Copyright (C) 2013 Laurent Montel <montel@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef SIEVEEDITORABSTRACTWIDGET_H
#define SIEVEEDITORABSTRACTWIDGET_H

#include <QWidget>

namespace KSieveUi {
class SieveEditorAbstractWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SieveEditorAbstractWidget(QWidget *parent=0);
    ~SieveEditorAbstractWidget();

    virtual QString currentscript();
    virtual void setImportScript( const QString & );

public Q_SLOTS:
    void slotSaveAs();
    void slotImport();

private:
    bool loadFromFile( const QString &filename );
};
}

#endif // SIEVEEDITORABSTRACTWIDGET_H
