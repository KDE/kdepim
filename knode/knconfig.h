/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2005 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef KNCONFIG_H
#define KNCONFIG_H

#include "knode_export.h"

#include <QList>
#include <QPixmap>
#include <QDateTime>
#include <kconfig.h>


class KNDisplayedHeader;


namespace KNode {

/** Base class for config settings.
 * @deprecated Use KConfigXT instead.
 */
class ConfigBase {

  public:
    ConfigBase() : d_irty(false) {}
    virtual ~ConfigBase()        {}

    virtual void save()    {}

    bool dirty()const           { return d_irty; }
    void setDirty(bool b)  { d_irty=b; }

  protected:
    bool d_irty;

};


/** Ex-Appearance settings.
 * @deprecated Move the remaining parts either to their only users or to the
 *  KConfigXT generated Settings class.
 */
class KNODE_EXPORT Appearance : public ConfigBase {

#define ICON_CNT 14

  friend class AppearanceWidget;

  public:
    enum IconIndex    { greyBall=0,        redBall=1,      greyBallChkd=2,
                        redBallChkd=3,     newFups=4,      eyes=5,
                        ignore=6,          mail=7,         posting=8,
                        canceledPosting=9, savedRemote=10, group=11,
                        null=12 };
    Appearance();

    const QPixmap& icon(IconIndex i)     { return i_cons[i]; }

  protected:
    void recreateLVIcons();

    QPixmap i_cons[ICON_CNT];

};


/** Headers displayed in the article viewer. */
class KNODE_EXPORT DisplayedHeaders : public ConfigBase
{
  public:
    DisplayedHeaders();
    ~DisplayedHeaders();

    void save();

    KNDisplayedHeader* createNewHeader();
    void remove(KNDisplayedHeader *h);
    void up(KNDisplayedHeader *h);
    void down(KNDisplayedHeader *h);

    /** Returns the list of headers displayed in the article viewer. */
    QList<KNDisplayedHeader*> headers() const { return mHeaderList; }


  protected:
    QList<KNDisplayedHeader*> mHeaderList;

};


/** Represents an additional header added by the composer. */
class KNODE_EXPORT XHeader
{
  public:
    /** Create a new XHeader object from the given string representation.
     * @param s String representation of a MIME header (i.e. "Name: Value").
     */
    explicit XHeader( const QString &s );

    /// A list of additional headers.
    typedef QList<XHeader> List;

    /** Returns the header name. */
    QString name() const { return mName; }
    /** Returns the header value. */
    QString value() const { return mValue; }
    /** Returns a string representation of the header, ie. "Name: Value". */
    QString header() const { return mName + ": " + mValue; }

  private:
    QString mName;
    QString mValue;
};



//BEGIN: Cleanup configuration -----------------------------------------------

/** Expirery/cleaup settings (globally or per account/group/folder). */
class KNODE_EXPORT Cleanup : public ConfigBase {

  friend class CleanupWidget;
  friend class GroupCleanupWidget;

  public:
    explicit Cleanup( bool global = true );
    ~Cleanup() {}

    void loadConfig( const KConfigGroup &conf );
    void saveConfig( KConfigGroup &conf );
    void save();

    //expire
    int maxAgeForRead() const        { return r_eadMaxAge; }
    int maxAgeForUnread() const      { return u_nreadMaxAge; }
    bool removeUnavailable() const   { return r_emoveUnavailable; }
    bool preserveThreads() const     { return p_reserveThr; }
    bool isGlobal() const            { return mGlobal; }
    bool useDefault() const          { return mDefault; }
    bool expireToday();
    void setLastExpireDate();

    void setUseDefault( bool def )   { mDefault = def; }

    //compact
    bool compactToday();
    void setLastCompactDate();


  protected:
    bool  d_oExpire,
          r_emoveUnavailable,
          p_reserveThr,
          d_oCompact;
    int   e_xpireInterval,
          r_eadMaxAge,
          u_nreadMaxAge,
          c_ompactInterval;

  private:
    /** global vs. per account or per group configuration */
    bool mGlobal;
    /** use default cleanup configuration */
    bool mDefault;
    /** last expiration and last comapction date */
    QDate mLastExpDate, mLastCompDate;

};

//END: Cleanup configuration -------------------------------------------------

} //KNode

#endif //KNCONFIG_H
