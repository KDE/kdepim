/*
    cryptoconfig.h

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#ifndef CRYPTOCONFIG_H
#define CRYPTOCONFIG_H

#include <kurl.h>

// Start reading this file from the bottom up :)

namespace Kleo {

  /**
   * Description of a single option
   */
  class CryptoConfigEntry {

  public:
    /**
       @li basic	This option should always be offered to the user.
       @li advanced	This option may be offered to advanced users.
       @li expert	This option should only be offered to expert users.
       */
    enum Level { Level_Basic = 0,
                 Level_Advanced = 1,
                 Level_Expert = 2 };

    /**
       Type of the argument
       @li ArgType_None	The option is set or not set, but no argument.
       @li ArgType_String	An unformatted string.
       @li ArgType_Int		A signed integer number.
       @li ArgType_UInt	An unsigned integer number.
       @li ArgType_Path	A string that describes the pathname of a file.
       The file does not necessarily need to exist.
       Separated from string so that e.g. a KURLRequester can be used.
       @li ArgType_URL		A URL
       @li ArgType_LDAPURL	A LDAP URL
       Separated from URL so that a more specific widget can be shown, hiding the url syntax
    */
    enum ArgType { ArgType_None = 0,
                   ArgType_String = 1,
                   ArgType_Int = 2,
                   ArgType_UInt = 3,
                   ArgType_Path = 4,
                   ArgType_URL = 5,
                   ArgType_LDAPURL = 6 };

    virtual ~CryptoConfigEntry() {}

    /**
     * @return user-visible description of this entry
     */
    virtual QString description() const = 0;

    /**
     * @return true if the argument is optional
     */
    virtual bool isOptional() const = 0;

    /**
     * @return true if the argument can be given multiple times
     */
    virtual bool isList() const = 0;

    /**
     * @return true if the argument can be changed at runtime
     */
    virtual bool isRuntime() const = 0;

    /**
     * User level
     */
    virtual Level level() const = 0;

    /**
     * Argument type
     */
    virtual ArgType argType() const = 0;

    /**
     * Return true if the option is set, i.e. different from default
     */
    virtual bool isSet() const = 0;

    /**
     * Return value as a bool (only allowed for ArgType_None)
     */
    virtual bool boolValue() const = 0;

    /**
     * Return value as a string (available for all argtypes)
     * The returned string can be empty (explicitely set to empty) or null (not set).
     */
    virtual QString stringValue() const = 0;

    /**
     * Return value as a signed int
     */
    virtual int intValue() const = 0;

    /**
     * Return value as an unsigned int
     */
    virtual unsigned int uintValue() const = 0;

    /**
     * Return value as a URL (only meaningful for Path and URL argtypes)
     */
    virtual KURL urlValue() const = 0;

    /**
     * Return number of times the option is set (only valid for ArgType_None, if isList())
     */
    virtual unsigned int numberOfTimesSet() const = 0;

    /**
     * Return value as a list of strings (mostly meaningful for String, Path and URL argtypes, if isList())
     */
    virtual QStringList stringValueList() const = 0;

    /**
     * Return value as a list of signed ints
     */
    virtual QValueList<int> intValueList() const = 0;

    /**
     * Return value as a list of unsigned ints
     */
    virtual QValueList<unsigned int> uintValueList() const = 0;

    /**
     * Return value as a list of URLs (only meaningful for Path and URL argtypes, if isList())
     */
    virtual KURL::List urlValueList() const = 0;

    /**
     * Reset an option to its default value
     */
    virtual void resetToDefault() = 0;

    /**
     * Define whether the option is set or not (only allowed for ArgType_None)
     * #### TODO: and for options with optional args
     */
    virtual void setBoolValue( bool ) = 0;

    /**
     * Set string value (allowed for all argtypes)
     */
    virtual void setStringValue( const QString& ) = 0;

    /**
     * Set a new signed int value
     */
    virtual void setIntValue( int ) = 0;

    /**
     * Set a new unsigned int value
     */
    virtual void setUIntValue( unsigned int ) = 0;

    /**
     * Set value as a URL (only meaningful for Path (if local) and URL argtypes)
     */
    virtual void setURLValue( const KURL& ) = 0;

    /**
     * Set the number of times the option is set (only valid for ArgType_None, if isList())
     */
    virtual void setNumberOfTimesSet( unsigned int ) = 0;

    /**
     * Set a new string-list value (only allowed for String, Path and URL argtypes, if isList())
     */
    virtual void setStringValueList( const QStringList& ) = 0;

    /**
     * Set a new list of signed int values
     */
    virtual void setIntValueList( const QValueList<int>& ) = 0;

    /**
     * Set a new list of unsigned int values
     */
    virtual void setUIntValueList( const QValueList<unsigned int>& ) = 0;

    /**
     * Set value as a URL list (only meaningful for Path (if all URLs are local) and URL argtypes, if isList())
     */
    virtual void setURLValueList( const KURL::List& ) = 0;

    /**
     * @return true if the value was changed
     */
    virtual bool isDirty() const = 0;
  };

  /**
   * Group containing a set of config options
   */
  class CryptoConfigGroup {

  public:
    virtual ~CryptoConfigGroup() {}

    /**
     * @return user-visible description of this entry
     */
    virtual QString description() const = 0;

    /**
     * User level
     */
    virtual CryptoConfigEntry::Level level() const = 0;

    /**
     * Returns the list of entries that are known by this group.
     *
     * @return list of group entry names.
     **/
    virtual QStringList entryList() const = 0;

    /**
     * @return the configuration object for a given entry in this group
     * The object is owned by CryptoConfigGroup, don't delete it.
     * Groups cannot be nested, so all entries returned here are pure entries, no groups.
     */
    virtual CryptoConfigEntry* entry( const QString& name ) const = 0;
  };

  /**
   * Crypto config for one component (e.g. gpg-agent, dirmngr etc.)
   */
  class CryptoConfigComponent {

  public:
    virtual ~CryptoConfigComponent() {}

    /**
     * Return user-visible description of this component
     */
    virtual QString description() const = 0;

    /**
     * Returns the list of groups that are known about.
     *
     * @return list of group names. One of them can be "<nogroup>", which is the group where all
     * "toplevel" options (belonging to no group) are.
     */
    virtual QStringList groupList() const = 0;

    /**
     * @return the configuration object for a given group
     * The object is owned by CryptoConfigComponent, don't delete it.
     */
    virtual CryptoConfigGroup* group( const QString& name ) const = 0;

  };

  /**
   * Main interface to crypto configuration.
   */
  class CryptoConfig {

  public:
    virtual ~CryptoConfig() {}

    /**
     * Returns the list of known components (e.g. "gpg-agent", "dirmngr" etc.).
     * Use @ref component() to retrieve more information about each one.
     * @return list of component names.
     **/
    virtual QStringList componentList() const = 0;

    /**
     * @return the configuration object for a given component
     * The object is owned by CryptoConfig, don't delete it.
     */
    virtual CryptoConfigComponent* component( const QString& name ) const = 0;

    /**
     * Convenience method to get hold of a single configuration entry when
     * its component, group and name are known. This can be used to read
     * the value and/or to set a value to it.
     *
     * @return the configuration object for a single configuration entry, 0 if not found.
     * The object is owned by CryptoConfig, don't delete it.
     */
    CryptoConfigEntry* entry( const QString& componentName, const QString& groupName, const QString& entryName ) const {
      const Kleo::CryptoConfigComponent* comp = component( componentName );
      const Kleo::CryptoConfigGroup* group = comp ? comp->group( groupName ) : 0;
      return group ? group->entry( entryName ) : 0;
    }

    /**
     * Write back changes
     *
     * @param runtime If this option is set, the changes will take effect at run-time, as
     * far as this is possible.  Otherwise, they will take effect at the next
     * start of the respective backend programs.
     */
    virtual void sync( bool runtime ) = 0;

    /**
     * Tells the CryptoConfig to discard any cached information, including
     * all components, groups and entries.
     * Call this to free some memory when you won't be using the object
     * for some time.
     * DON'T call this if you're holding pointers to components, groups or entries.
     */
    virtual void clear() = 0;
  };

}

#endif /* CRYPTOCONFIG_H */
