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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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

ResourceBlogging::ResourceBlogging( const KConfig *config )
  : ResourceGroupwareBase( config )
{
  init();
  if ( config ) readConfig( config );
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

void ResourceBlogging::readConfig( const KConfig *config )
{
  BloggingCalendarAdaptor *ad = dynamic_cast<BloggingCalendarAdaptor*>( adaptor() );
  ResourceGroupwareBase::readConfig( config );
  if ( ad && prefs() ) {
    ad->setUser( prefs()->user() );
    ad->setPassword( prefs()->password() );
    ad->setBaseURL( prefs()->url() );
  }
//   QString url = config->readEntry( "URL" );
//   mUrl = KURL( url );
  
//   mServerAPI = config->readNumEntry( "ServerAPI" );
//   mTemplate.setCategoryTagOpen( config->readEntry( "CategoryTagOpen", "<CATEGORY>" ) ); 
//   mTemplate.setCategoryTagClose( config->readEntry( "CategoryTagClose", "</CATEGORY>" ) );
//   mTemplate.setTitleTagOpen( config->readEntry( "TitleTagOpen", "<TITLE>" ) );
//   mTemplate.setTitleTagClose( config->readEntry( "TitleTagClose", "</TITLE>" ) );

}

void ResourceBlogging::writeConfig( KConfig *config )
{
  kdDebug(5800) << "ResourceBlogging::writeConfig()" << endl;

  ResourceCalendar::writeConfig( config );

//   config->writeEntry( "URL", mUrl.url() );
//   config->writeEntry( "ServerAPI", mServerAPI );
//   config->writeEntry( "CategoryTagOpen", mTemplate.categoryTagOpen() );
//   config->writeEntry( "CategoryTagClose", mTemplate.categoryTagClose() );
//   config->writeEntry( "TitleTagOpen", mTemplate.titleTagOpen() );
//   config->writeEntry( "TitleTagClose", mTemplate.titleTagClose() );

  ResourceGroupwareBase::writeConfig( config );
}


#include "kcal_resourceblogging.moc"
