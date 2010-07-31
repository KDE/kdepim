/*
 *  colourcombo.h  -  colour selection combo box
 *  Program:  kalarm
 *  Copyright © 2001-2003,2005,2006 by David Jarvie <software@astrojar.org.uk>
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
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef COLOURCOMBO_H
#define COLOURCOMBO_H

#include <tqcombobox.h>
#include "colourlist.h"


/**
 *  @short A colour selection combo box whose colour list can be specified.
 *
 *  The ColourCombo class is a combo box allowing the user to select a colour.
 *
 *  It is similar to KColorCombo but allows the list of colours to be restricted to those
 *  which are specified. The first item in the list is a custom colour entry, which allows
 *  the user to define an arbitrary colour. The remaining entries in the list are preset
 *  by the program.
 *
 *  The widget may be set as read-only. This has the same effect as disabling it, except
 *  that its appearance is unchanged.
 *
 *  @author David Jarvie <software@astrojar.org.uk>
 */
class ColourCombo : public QComboBox
{
		Q_OBJECT
		Q_PROPERTY(TQColor color READ color WRITE setColor)
	public:
		/** Constructor.
		 *  @param parent The parent object of this widget.
		 *  @param name The name of this widget.
		 *  @param defaultColour The colour which is selected by default.
		 */
		explicit ColourCombo(TQWidget* parent = 0, const char* name = 0, const TQColor& defaultColour = 0xFFFFFF);
		/** Returns the selected colour. */
		TQColor       color() const               { return mSelectedColour; }
		/** Returns the selected colour. */
		TQColor       colour() const              { return mSelectedColour; }
		/** Sets the selected colour to @p c. */
		void         setColor(const TQColor& c)   { setColour(c); }
		/** Sets the selected colour to @p c. */
		void         setColour(const TQColor& c);
		/** Initialises the list of colours to @p list. */
		void         setColours(const ColourList& list);
		/** Returns true if the first entry in the list, i.e. the custom colour, is selected. */
		bool         isCustomColour() const      { return !currentItem(); }
		/** Returns true if the widget is read only. */
		bool         isReadOnly() const          { return mReadOnly; }
		/** Sets whether the combo box can be changed by the user.
		 *  @param readOnly True to set the widget read-only, false to set it read-write.
		 */
		virtual void setReadOnly(bool readOnly);
	signals:
		/** Signal emitted when a new colour has been selected. */
		void         activated(const TQColor&);    // a new colour box has been selected
		/** Signal emitted when a new colour has been highlighted. */
		void         highlighted(const TQColor&);  // a new item has been highlighted
	public slots:
		/** Enables or disables the widget. */
		virtual void setEnabled(bool enabled);
	protected:
		virtual void resizeEvent(TQResizeEvent*);
		virtual void mousePressEvent(TQMouseEvent*);
		virtual void mouseReleaseEvent(TQMouseEvent*);
		virtual void mouseMoveEvent(TQMouseEvent*);
		virtual void keyPressEvent(TQKeyEvent*);
		virtual void keyReleaseEvent(TQKeyEvent*);
	private slots:
		void         slotActivated(int index);
		void         slotHighlighted(int index);
		void         slotPreferencesChanged();
	private:
		void         addColours();
		void         drawCustomItem(TQRect&, bool insert);

		ColourList   mColourList;      // the sorted colours to display
		TQColor       mSelectedColour;  // currently selected colour
		TQColor       mCustomColour;    // current colour of the Custom item
		bool         mReadOnly;        // value cannot be changed
		bool         mDisabled;
};

#endif // COLOURCOMBO_H
