/*
    This file is part of KOrganizer.

    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef KOPREFS_H
#define KOPREFS_H

#include <tqdict.h>

#include <kdepimmacros.h>

#include "koprefs_base.h"

class KConfig;
class TQFont;
class TQColor;
class TQStringList;

TQColor getTextColor(const TQColor &c);


class KDE_EXPORT KOPrefs : public KOPrefsBase
{
  public:
    virtual ~KOPrefs();

    /** Get instance of KOPrefs. It is made sure that there is only one
    instance. */
    static KOPrefs *instance();

    /** Set preferences to default values */
    void usrSetDefaults();

    /** Read preferences from config file */
    void usrReadConfig();

    /** Write preferences to config file */
    void usrWriteConfig();

  protected:
    void setCategoryDefaults();
    void setTimeZoneIdDefault();

    /** Fill empty mail fields with default values. */
    void fillMailDefaults();

  private:
    /** Constructor disabled for public. Use instance() to create a KOPrefs
    object. */
    KOPrefs();

    static KOPrefs *mInstance;

  public:
    // preferences data
    void setFullName( const TQString & );
    TQString fullName();
    void setEmail( const TQString & );
    TQString email();
    /// Returns all email addresses for the user.
    TQStringList allEmails();
    /// Returns all email addresses together with the full username for the user.
    TQStringList fullEmails();
    /// Return true if the given email belongs to the user
    bool thatIsMe( const TQString& email );

    void setCategoryColor( const TQString &cat, const TQColor &color );
    TQColor *categoryColor( const TQString &cat );
    bool hasCategoryColor( const TQString &cat ) const;

    void setResourceColor ( const TQString &, const TQColor & );
    TQColor* resourceColor( const TQString & );

    TQString mHtmlExportFile;

    // Groupware passwords
    TQString mPublishPassword;
    TQString mRetrievePassword;

  private:
    TQDict<TQColor> mCategoryColors;
    TQColor mDefaultCategoryColor;

    TQDict<TQColor> mResourceColors;
    TQColor mDefaultResourceColor;

    TQFont mDefaultTimeBarFont;
    TQFont mDefaultMonthViewFont;

    TQStringList mMyAddrBookMails;

  public: // Do not use - except in KOPrefsDialogMain
    TQString mName;
    TQString mEmail;
};

#endif
