/*
* Copyright (C) 2005, Mart Kelder (mart.kde@hccnet.nl)
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#ifndef MK_ACCOUNT_INPUT
#define MK_ACCOUNT_INPUT

class QWidget;
class QString;
class QStringList;

class QLabel;
class KLineEdit;
class KURLRequester;
class QComboBox;
class QCheckBox;

#include <qmap.h>

/**
 * This classe contains methods to use in the creation of the protocol configuration box.
 * The subclasses of this classes define the methods.
 */
class AccountInput
{
public:
	/**
	 * Constructor.
	 *
	 * @param configName The name as the information is stored in the configuration-file.
	 */
	AccountInput( const QString& configName );
	/**
	 * Destructor
	 */
	virtual ~AccountInput();

	/**
	 * Implementations should return the left widget. In the configuration, it is
	 * possible to make a left label and a right input box.
	 * @return The pointer to the widget.
	 */
	virtual QWidget* leftWidget() = 0;
	/**
	 * Implementations should return the right widget. In the configuration, it is
	 * possible to make a left label and a right input box.
	 * @return The pointer to the widget.
	 */
	virtual QWidget* rightWidget() = 0;

	/**
	 * This function return the config name. This is used for stored an retrieving information.
	 *
	 * @return The configName as used for this config field.
	 */
	QString configName() const;
	
	/**
	 * Return the value of the configuration object. In most cases, this is the value the user typed in.
	 * This information will be stored in the configuration file.
	 *
	 * @return The value of the object
	 */
	virtual QString value() const = 0;
	
	/**
	 * Implementation should edit there widget such that value() would return the parameter.
	 *
	 * @param value The value that the object must get.
	 */
	virtual void setValue( const QString& value ) = 0;
protected:
	QString *_configName;
};

/**
 * This class implement a simple text input.
 * The left widget is a label, the right widget is a KLineEdit.
 * The value of this object is determined by the value of the KLineEdit.
 */
class TextInput : public AccountInput
{
public:
	/** 
	 * Enum for specifing the type.
	 * text means a normal LineEdit,
	 * password means that *-sings are used instead of characters.
	 */
	enum Type { text, password };
	
	/**
	 * Constructor
	 *
	 * @param parent The parent widget
	 * @param title The title that appears on the screen
	 * @param type The type of TextEdit which is used
	 * @param defaul The default value of this object
	 * @param configName The name it has in the configuration box.
	 */
	TextInput( QWidget *parent, const QString& title, Type type, const QString& defaul, const QString& configName );
	/**
	 * Constructor. Use this one if you want to ensure a number is inserted.
	 *
	 * @param parent The parent widget
	 * @param title The title that appears on the screen
	 * @param min The minimum value that can be inserted
	 * @param max The maximum value that can be inserted
	 * @param defaul The default value of this object
	 * @param configName The name it has in the configuration box.
	 */
	TextInput( QWidget *parent, const QString& title, int min, int max, const QString& defaul, const QString& configName );
	/**
	 * Destructor
	 */
	virtual ~TextInput();

	/**
	 * Returns a pointer to the label.
	 * @return A pointer to the label
	 */
	virtual QWidget* leftWidget() { return (QWidget*)_left; }
	/**
	 * Returns a pointer to the KLineEdit.
	 * @return A pointer to the KLineEdit
	 */
	virtual QWidget* rightWidget() { return (QWidget*)_right; }

	/**
	 * The value of the lineedit.
	 * @return The value of the lineedit.
	 */
	virtual QString value() const;
	/**
	 * This function sets the text of the edit box.
	 * @param value The value to appear in the lineedit box.
	 */
	virtual void setValue( const QString& value );
private:
	QLabel *_left;
	KLineEdit *_right;
};

/**
 * This class implements a URL AccountInput. It can be used to request a file.
 */
class URLInput : public AccountInput
{
public:
	/**
	 * Constructor
	 * @param parent The parent of this object
	 * @param title The title of the label next to the URL.
	 * @param defaul The default value
	 * @param configName The name of the configuration entry
	 */
	URLInput( QWidget *parent, const QString& title, const QString& defaul, const QString& configName );
	/**
	 * Destructor
	 */
	virtual ~URLInput();

	/**
	 * This return a pointer to the label of this object
	 * @return A pointer to the label of this object
	 */
	virtual QWidget* leftWidget() { return (QWidget*)_left; }
	/**
	* This return a pointer to the KURLRequestor of this object
	* @return A pointer to the KURLRequestor of this object
	 */
	virtual QWidget* rightWidget() { return (QWidget*)_right; }

	/**
	 * This function returns the url as given in the KURLRequestor
	 * @return The currently selected url
	 */
	virtual QString value() const;
	/**
	 * Sets the currently selected url
	 * @param value The url to be set.
	 */
	virtual void setValue( const QString& );

private:
	QLabel *_left;
	KURLRequester *_right;
};

/**
 * This is an imput for a combobox.
 */
class ComboInput : public AccountInput
{
public:
	/** 
	 * Constructor
	 *
	 * @param parent The parent of the widgets which are created
	 * @param title The title next to the combo box
	 * @param list A mapping which maps a value in the configuration to a (translated) entry in the
	 *             combo box.
	 * @param default The default value of the combo box.
	 * @param configName The name in which the option is saved.
	 */
	ComboInput( QWidget *parent, const QString& title, const QMap<QString,QString>& list,
	            const QString& defaul, const QString& configName );
	/**
	 * Destructor
	 */
	virtual ~ComboInput();

	/**
	 * The left widget (a label with the title on it)
	 * @return A pointer to the label of this object.
	 */
	virtual QWidget* leftWidget() { return (QWidget*)_left; }
	/**
	 * The right widget (the combo box itselfs)
	 * @return A pointer to the combo box of this object
	 */
	virtual QWidget* rightWidget() { return (QWidget*)_right; }

	/**
	 * Return the value of the currently selected item
	 * @return The value of the currently selected item
	 */
	virtual QString value() const;
	
	/**
	 * This function sets the combo box to an item which has @p value as value.
	 *
	 * @param value The value to be searched
	 */
	virtual void setValue( const QString& value );
private:
	QLabel *_left;
	QComboBox *_right;
	QMap< QString, QString > *_list;

};

/**
 * This is an object for creating a text-box.
 * If has no left widget, as the title is stored in the checkbox itselfs.
 */
class CheckboxInput : public AccountInput
{
public:
	/**
	 * Constructor
	 *
	 * @param parent The parent for the objects which are created
	 * @param title The title of the checkbox
	 * @param defaul The default value ("true" for checked, "false" otherwise")
	 * @param configName The name of the configuration entry of this object
	 */
	CheckboxInput( QWidget *parent, const QString& title, const QString& defaul, const QString& configName );
	/**
	 * Destructor
	 */
	virtual ~CheckboxInput();

	/**
	 * Return a 0-pointer as this object doesn't have a left widget.
	 *
	 * @return 0
	 */
	virtual QWidget* leftWidget() { return 0; }
	/**
	 * This function returns the checkbox.
	 * @return A pointer to the checkbox.
	 */
	virtual QWidget* rightWidget() { return (QWidget*)_right; }

	/**
	 * This gives the value of the checkbox: "true" if checked, "false" otherwise.
	 *
	 * @return "true" if the checkbox is checked, "false" otherwise.
	 */
	virtual QString value() const;
	/**
	 * This function can change the state of the checkbox.
	 * It can check or uncheck it.
	 * 
	 * @param value If this parameter is "true", the checkbox gets checked,
	 *              if it is "false", the checkbox get unchecked.
	 */
	virtual void setValue( const QString& value );

private:
	QCheckBox *_right;
};

#endif

