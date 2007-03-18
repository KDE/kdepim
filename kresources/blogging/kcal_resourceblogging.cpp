/*
    This file is part of kdepim.

    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "bloggingcalendaradaptor.h"
#include "API_Blogger.h"

#include "kcal_resourceblogging.h"
#include <kresources_groupwareprefs.h>

using namespace KCal;
    
KBlog::APIBlog *ResourceBlogging::mAPI = 0;

ResourceBlogging::ResourceBlogging()
  : ResourceGroupwareBase()
{
  init();
}

ResourceBlogging::ResourceBlogging( const KConfigGroup &group )
  : ResourceGroupwareBase( group )
{
  init();
  readConfig( group );
}

void ResourceBlogging::init()
{
  setType( "ResourceBlogging" );
  setPrefs( createPrefs() );
  setFolderLister( new KPIM::FolderLister( KPIM::FolderLister::Calendar ) );
  BloggingCalendarAdaptor *ad = new BloggingCalendarAdaptor();
  setAdaptor( ad );
  ad->setAPI( new KBlog::APIBlogger( prefs()->url(), this ) );
  
  ResourceGroupwareBase::init();
}

void ResourceBlogging::readConfig( const KConfigGroup &group )
{
  BloggingCalendarAdaptor *ad = dynamic_cast<BloggingCalendarAdaptor*>( adaptor() );
  ResourceGroupwareBase::readConfig( group );
  if ( ad && prefs() ) {
    ad->setUser( prefs()->user() );
    ad->setPassword( prefs()->password() );
    ad->setBaseURL( prefs()->url() );
  }
//   QString url = group.readEntry( "URL" );
//   mUrl = KUrl( url );
  
//   mServerAPI = group.readEntry( "ServerAPI" );
//   mTemplate.setCategoryTagOpen( group.readEntry( "CategoryTagOpen", "<CATEGORY>" ) ); 
//   mTemplate.setCategoryTagClose( group.readEntry( "CategoryTagClose", "</CATEGORY>" ) );
//   mTemplate.setTitleTagOpen( group.readEntry( "TitleTagOpen", "<TITLE>" ) );
//   mTemplate.setTitleTagClose( group.readEntry( "TitleTagClose", "</TITLE>" ) );

}

void ResourceBlogging::writeConfig( KConfigGroup &group )
{
  kDebug(5800) << "ResourceBlogging::writeConfig()" << endl;

  ResourceCalendar::writeConfig( group );

//   group.writeEntry( "URL", mUrl.url() );
//   group.writeEntry( "ServerAPI", mServerAPI );
//   group.writeEntry( "CategoryTagOpen", mTemplate.categoryTagOpen() );
//   group.writeEntry( "CategoryTagClose", mTemplate.categoryTagClose() );
//   group.writeEntry( "TitleTagOpen", mTemplate.titleTagOpen() );
//   group.writeEntry( "TitleTagClose", mTemplate.titleTagClose() );

  ResourceGroupwareBase::writeConfig( group );
}


#include "kcal_resourceblogging.moc"
