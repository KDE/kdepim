/*                                                                      
    This file is part of KAddressBook.                                  
    Copyright (c) 2002 Mike Pilone <mpilone@slac.com>                   
                                                                        
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or   
    (at your option) any later version.                                 
                                                                        
    This program is distributed in the hope that it will be useful,     
    but WITHOUT ANY WARRANTY; without even the implied warranty of      
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the        
    GNU General Public License for more details.                        
                                                                        
    You should have received a copy of the GNU General Public License   
    along with this program; if not, write to the Free Software         
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.           
                                                                        
    As a special exception, permission is given to link this program    
    with any edition of Qt, and distribute the resulting executable,    
    without including the source code for Qt in the source distribution.
*/                                                                      

#ifndef FILTER_H
#define FILTER_H

#include <qstring.h>
#include <qstringlist.h>
#include <qvaluelist.h>

#include <kabc/addressee.h>
#include <kconfig.h>

/**
  Filter for AddressBook related objects (Addressees)

  @todo This class should be switched to use shared data.
 */
class Filter
{
  public:
    typedef QValueList<Filter> List;

    enum MatchRule { Matching = 0, NotMatching = 1 };

    Filter();
    Filter( const QString& name );
    ~Filter();

    /**
      Set the name of the filter.
     */
    void setName( const QString &name );

    /**
      @return The name of the filter.
     */
    const QString &name() const;

    /**
      @return Whether the filter is an internal one.
     */
    bool isInternal() const;

    /**
      Apply the filter to the addressee list. All addressees not passing
      the filter criterias will be removed from the list.

      If the MatchRule is NotMatch, then all the addressees matching the
      filter will be removed from the list.
     */
    void apply( KABC::Addressee::List &addresseeList );

    /**
      Apply the filter to the addressee.

      @return True if the addressee passes the criteria, false otherwise.
      The return values are opposite if the MatchRule is NotMatch.
     */
    bool filterAddressee( const KABC::Addressee &a );

    /**
      Enable or disable the filter
     */
    void setEnabled( bool on );

    /**
      @return True if this filter is enabled, false otherwise.
     */
    bool isEnabled() const;

    /**
      Set the list of categories. This list is used to filter addressees.
     */
    void setCategories( const QStringList &list );

    /**
      @return The list of categories.
     */
    const QStringList &categories() const;

    /**
      Saves the filter to the config file. The group should already be set.
     */
    void save( KConfig *config );

    /**
      Loads the filter from the config file. The group should already be set.
     */
    void restore( KConfig *config );

    /**
      Saves a list of filters to the config file.

      @param config The config file to use
      @param baseGroup The base groupname to use. The number of filters
                       will be written to this group, then a _1, _2, etc
                       will be append for each filter saved.
      @param list The list of filters to be saved.
     */
    static void save( KConfig *config, const QString &baseGroup, Filter::List &list );

    /**
      Restores a list of filters from a config file.

      @param config The config file to read from.
      @param baseGroup The base group name to be used to find the filters

      @return The list of filters.
     */
    static Filter::List restore( KConfig *config, const QString &baseGroup );

    /**
      Sets the filter rule. If the rule is Filter::Matching (default),
      then the filter will return true on items that match the filter.
      If the rule is Filter::NotMatching, then the filter will return
      true on items that do not match the filter.
     */
    void setMatchRule( MatchRule rule );

    /** @return The current match rule.
    */
    MatchRule matchRule() const;

  private:
    QString mName;
    QStringList mCategoryList;
    MatchRule mMatchRule;
    bool mEnabled;
    bool mInternal;
};

#endif
