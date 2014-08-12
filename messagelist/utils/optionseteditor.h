/******************************************************************************
 *
 *  Copyright 2008 Szymon Tomasz Stefanek <pragma@kvirc.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *******************************************************************************/

#ifndef __MESSAGELIST_UTILS_OPTIONSETEDITOR_H__
#define __MESSAGELIST_UTILS_OPTIONSETEDITOR_H__

#include <QTabWidget>

class KLineEdit;
class KTextEdit;

namespace MessageList
{

namespace Core
{

class OptionSet;

} // namespace Core

namespace Utils
{

/**
 * The base class for the OptionSet editors. Provides common functionality.
 */
class OptionSetEditor : public QTabWidget
{
    Q_OBJECT

public:
    explicit OptionSetEditor( QWidget *parent );
    ~OptionSetEditor();
    void setReadOnly( bool readOnly );

protected:

    /**
   * Returns the editor for the name of the OptionSet.
   * Derived classes are responsable of filling this UI element and reading back data from it.
   */
    KLineEdit * nameEdit() const;

    /**
   * Returns the editor for the description of the OptionSet.
   * Derived classes are responsable of filling this UI element and reading back data from it.
   */
    KTextEdit * descriptionEdit() const;

protected slots:
    /**
   * Handles editing of the name field.
   * Pure virtual slot. Derived classes must provide an implementation.
   */
    virtual void slotNameEditTextEdited( const QString &newName ) = 0;

private:
    KLineEdit * mNameEdit;                       ///< The editor for the OptionSet name
    KTextEdit * mDescriptionEdit;                ///< The editor for the OptionSet description

};

} // namespace Utils

} // namespace MessageList

#endif //!__MESSAGELIST_UTILS_OPTIONSETEDITOR_H__
