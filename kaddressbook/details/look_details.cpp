/*                                                                      
    This file is part of KAddressBook.
    Copyright (c) 1996-2002 Mirko Boehm <mirko@kde.org>
                                                                        
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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.           
                                                                        
    As a special exception, permission is given to link this program    
    with any edition of Qt, and distribute the resulting executable,    
    without including the source code for Qt in the source distribution.
*/                                                                      

#include <kconfig.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <kinstance.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include <qcursor.h>
#include <qdir.h>
#include <qpainter.h>
#include <qpopupmenu.h>

#include "global.h"
#include "kabentrypainter.h"

#include "look_details.h"

#define GRID 5

const QString KABDetailedView::BorderedBGDir="kab3part/backgrounds/bordered/";
const QString KABDetailedView::TiledBGDir="kab3part/backgrounds/tiled/";

KABDetailedView::KABDetailedView(QWidget* parent, const char* name)
    : KABBasicLook(parent, name),
      epainter(0),
      bgStyle(None),
      defaultBGColor(white),
      headlineBGColor(darkBlue),
      headlineTextColor(yellow),
      Grid(3),
      menuBorderedBG(0),
      menuTiledBG(0)
{
    KToggleAction** actions[]= {
        &actionShowAddresses,
        &actionShowEmails,
        &actionShowTelephones,
        &actionShowURLs
    };
    QString texts[]= {
        i18n("Show Postal Addresses"),
        i18n("Show Email Addresses"),
        i18n("Show Telephone Numbers"),
        i18n("Show Web Pages (URLs)")
    };
    QFont general=KGlobalSettings::generalFont();
    QFont fixed=KGlobalSettings::fixedFont();
    QString gfont=general.family();
    QString ffont=fixed.family();
    int gpointsize=general.pixelSize();
    if ( gpointsize == -1 )
      gpointsize = general.pointSize();

    int fpointsize=fixed.pixelSize();
    if ( fpointsize == -1 )
      fpointsize = fixed.pointSize();

    epainter=new KABEntryPainter
             (Qt::black, headlineTextColor,
              useHeadlineBGColor, headlineBGColor,
              QFont(gfont, gpointsize+4, QFont::Bold, true),
              QFont(gfont, gpointsize+2, QFont::Bold, true),
              QFont(gfont, gpointsize, QFont::Normal, false),
              QFont(ffont, fpointsize, QFont::Normal, false),
              QFont(gfont, gpointsize, QFont::Normal, false));
    const int Size=sizeof(actions)/sizeof(actions[0]);
    // ----- create some actions:
    for(int count=0; count<Size; ++count)
    {
        *actions[count]=new KToggleAction(texts[count]);
        (*actions[count])->setChecked(true);
    }
    // ----- we would like to track mouse movement:
    setMouseTracking(true);
    /*
    // ----- find preferred size:
    // Since the detailed look does not prefer a size it uses the
    // preferred size of the editing look to avoid flickering resizing.
    KABEditLook *editlook=new KABEditLook(api, this);
    setMinimumSize(editlook->minimumSizeHint());
    delete editlook;
    */
}

KABDetailedView::~KABDetailedView()
{
    if(epainter!=0) delete epainter;
}

bool KABDetailedView::getBackground(QString path, QPixmap& image)
{
    QMap<QString, QPixmap>::iterator pos;
    pos=backgrounds.find(path);
    if(pos==backgrounds.end())
    { // the image has not been loaded previously
        if(image.load(path))
	{
            backgrounds[path]=image;
            return true;
	} else {
            return false;
	}
    } else {
        // image found in cache
        image=pos.data();
        return true;
    }
}

void KABDetailedView::paintEvent(QPaintEvent*)
{
    const int BorderSpace=Grid;
    QPixmap pm(width(), height());
    QPainter p;
    QRect entryArea=QRect(BorderSpace, Grid, width()-Grid-BorderSpace,
                          height()-2*Grid);
    p.begin(&pm);
    // ----- load the background pattern, or clear the painter:
    p.setPen(darkBlue);
    p.setBrush(defaultBGColor);
    p.drawRect(0, 0, width(), height());
    switch(bgStyle)
    {
    case Tiled:
        p.drawTiledPixmap(1, 1, width()-2, height()-2, background);
        break;
    case Bordered:
        p.drawTiledPixmap(1, 1,
                          QMIN(width()-2,
                               background.width()),
                          height()-2, background);
        break;
    case None: // no BG image defined for this entry:
    default:
        if(useDefaultBGImage)
        {
            p.drawTiledPixmap(1, 1, width()-2, height()-2, defaultBGImage);
        }
        break;
    };
    p.setViewport(entryArea);
    epainter->setShowAddresses(actionShowAddresses->isChecked());
    epainter->setShowEmails(actionShowEmails->isChecked());
    epainter->setShowTelephones(actionShowTelephones->isChecked());
    epainter->setShowURLs(actionShowURLs->isChecked());
    epainter->printEntry( addressee(),
                         QRect(0, 0, entryArea.width(), entryArea.height()),
                         &p);
    p.end();
    bitBlt(this, 0, 0, &pm);
}

void KABDetailedView::mouseMoveEvent(QMouseEvent *e)
{
    QPoint bias(Grid, Grid);
    int rc;
    bool hit=false;
    // -----
    if((rc=epainter->hitsEmail(e->pos()-bias))!=-1)
    {
        //       kdDebug() << "KABDetailedView::mouseMoveEvent: "
        // << "pointer touches email "
        // 		<< rc << endl;
        hit=true;
    }
    else
        if((rc=epainter->hitsURLs(e->pos()-bias))!=-1)
        {
            // 	kdDebug() << "KABDetailedView::mouseMoveEvent: "
            // << "pointer touches URL "
            // 		  << rc << endl;
            hit=true;
        }
        else
            if((rc=epainter->hitsTelephones(e->pos()-bias))!=-1)
            {
                // 	  kdDebug() << "KABDetailedView::mouseMoveEvent: "
                // << "pointer touches telephone no. "
                // 		    << rc << endl;
                hit=true;
            }
            else
                if((rc=epainter->hitsTalkAddresses(e->pos()-bias))!=-1)
                {
                    // 	    kdDebug() << "KABDetailedView::mouseMoveEvent: "
                    // << "pointer touches talk address "
                    // 		      << rc << endl;
                    hit=true;
                }
    if(hit)
    {
        if(cursor().shape()!=PointingHandCursor)
	{
            setCursor(PointingHandCursor);
	}
    } else {
        if(cursor().shape()!=ArrowCursor)
	{
            setCursor(ArrowCursor);
	}
    }
}

void KABDetailedView::mousePressEvent(QMouseEvent *e)
{
    QPopupMenu menu(this);
    QPopupMenu *menuBG=new QPopupMenu(&menu);
    menuBorderedBG=new QPopupMenu(&menu);
    menuTiledBG=new QPopupMenu(&menu);
    menu.insertItem(i18n("Select Background"), menuBG);
    menuBG->insertItem(i18n("Bordered Backgrounds"), menuBorderedBG);
    menuBG->insertItem(i18n("Tiled Backgrounds"), menuTiledBG);
    menu.insertSeparator();
    QPoint point=e->pos()-QPoint(Grid, Grid);
    int rc;
    QStringList dirsBorderedBG, dirsTiledBG;
    QDir dir;
    // -----
    switch(e->button())
    {
    case QMouseEvent::RightButton:
        if(isReadOnly())
        {
            menu.setItemEnabled(menu.idAt(0), false);
        } else {
            // @todo: settings need to be saved in view options
            // ----- load background options:
            dirsBorderedBG=KGlobal::instance()->dirs()->findDirs
                           ("data", BorderedBGDir);
            if(dirsBorderedBG.count()>0)
            {
                dir.setPath(dirsBorderedBG[0]);
                borders=dir.entryList(QDir::Files);
                for(unsigned count=0; count<borders.count(); ++count)
                {
                    menuBorderedBG->insertItem(borders[count], count);
                }
                connect(menuBorderedBG, SIGNAL(activated(int)),
                        this, SLOT(slotBorderedBGSelected(int)));
            } else {
                menuBG->setItemEnabled(menuBG->idAt(0), false);
            }
            dirsTiledBG=KGlobal::instance()->dirs()->findDirs
                        ("data", TiledBGDir);
            if(dirsTiledBG.count()>0)
            {
                dir.setPath(dirsTiledBG[0]);
                tiles=dir.entryList(QDir::Files);
                for(unsigned count=0; count<tiles.count(); ++count)
                {
                    menuTiledBG->insertItem(tiles[count], count);
                }
                connect(menuTiledBG, SIGNAL(activated(int)),
                        this, SLOT(slotTiledBGSelected(int)));
            } else {
                menuBG->setItemEnabled(menuBG->idAt(1), false);
            }
        }
        // ----- done, plug actions:
        actionShowAddresses->plug(&menu);
        actionShowEmails->plug(&menu);
        actionShowTelephones->plug(&menu);
        actionShowURLs->plug(&menu);
        // ----- done, execute menu:
        menu.exec(e->globalPos());
        break;
    case QMouseEvent::LeftButton:
        // ----- find whether the pointer touches an email address, URL,
        // talk address or telephone number:
        if((rc=epainter->hitsEmail(point))!=-1)
	{
            emit(sendEmail( addressee().emails()[rc]));
            break;
	}
        if((rc=epainter->hitsURLs(point))!=-1)
	{
            emit(browse( addressee().url().prettyURL()));
            break;
	}
        if((rc=epainter->hitsTelephones(point))!=-1)
	{
            /* emit(call(addressee().telephone.at(2*rc),
               addressee().telephone.at(2*rc+1))); */
            kdDebug(5720) << "KABDetailedView::mousePressEvent: ni (calling)."
                      << endl;
            break;
	}
        if((rc=epainter->hitsTalkAddresses(point))!=-1)
	{
            /* emit(talk(addressee().talk.at(rc))); */
            kdDebug(5720) << "KABDetailedView::mousePressEvent: ni (invoking ktalk)."
                      << endl;
            break;
	}
        kdDebug(5720) << "KABDetailedView::mousePressEvent: not over active item."
                  << endl;
        break;
    default:
        break;
    };
    menuBorderedBG=0;
    menuTiledBG=0;
}

void KABDetailedView::setAddressee(const KABC::Addressee& e)
{

    BackgroundStyle style=None;
    QString dir, file, styleSetting;
    KABBasicLook::setAddressee(e);
    // @todo: preload path and styleSetting with possible preference values
    // ----- load the background image:
    styleSetting=addressee().custom("kab", "BackgroundStyle");
    style=(BackgroundStyle)styleSetting.toInt();
    file=addressee().custom("kab", "BackgroundImage");
    if(!file.isEmpty())
    {
        switch(style)
        {
        case Tiled:
            dir=TiledBGDir;
            break;
        case Bordered:
            dir=BorderedBGDir;
            break;
        case None:
        default:
            break;
        }
        // ----- path is located under KDEDIR/share:
        QStringList dirs;
        dirs=KGlobal::instance()->dirs()->findDirs("data", dir);
        bgStyle=None;
        if(!dirs.isEmpty())
        {
            unsigned count;
            for(count=0; count<dirs.count(); ++count)
            {
                QDir folder;
                folder.setPath(dirs[count]);
                file=folder.absPath()+"/"+file;
                if(getBackground(file, background))
                {
                    bgStyle=style;
                    break;
                }
            }
            if(count==dirs.count())
            {   // not found:
                kdDebug(5720) << "KABDetailedView::setEntry: " << file
                          << " not locatable." << endl;
            }
        }
    } else { // no background here:
        bgStyle=None;
        background.resize(0,0);
    }
    repaint(false);
}

void KABDetailedView::slotBorderedBGSelected(int index)
{
    if(index>=0 && (unsigned)index<borders.count() && !isReadOnly())
    {
        // ----- get the selection and make it a full path:
        QString path=borders[index];
        bgStyle=Bordered;
        addressee().insertCustom("kab", "BackgroundStyle", QString().setNum(bgStyle));
        addressee().insertCustom("kab", "BackgroundImage", path);
        setAddressee( addressee() );
        emit(entryChanged());
    }
}

void KABDetailedView::slotTiledBGSelected(int index)
{
    if(index>=0 && (unsigned)index<tiles.count() && !isReadOnly())
    {
        QString path=tiles[index];
        bgStyle=Tiled;
        addressee().insertCustom("kab", "BackgroundStyle", QString().setNum(bgStyle));
        addressee().insertCustom("kab", "BackgroundImage", path);
        setAddressee( addressee() );
        emit(entryChanged());
    }
}


void KABDetailedView::setReadOnly( bool state )
{
  KABBasicLook::setReadOnly( state );
  repaint( false );
}

void KABDetailedView::restoreSettings( KConfig *config )
{
    QFont general=KGlobalSettings::generalFont();
    QFont fixed=KGlobalSettings::fixedFont();
    QString gfont=general.family();
    QString ffont=fixed.family();

    int gpointsize=general.pixelSize();
    if ( gpointsize == -1 )
      gpointsize = general.pointSize();

    int fpointsize=fixed.pixelSize();
    if ( fpointsize == -1 )
      fpointsize = fixed.pointSize();

    // -----
    bool useBGImage=true;
    config->setGroup(ConfigView);
    // ----- load the default background image:
    QString bgImage;
    useDefaultBGImage=config->readBoolEntry
                      (ConfigView_UseDefaultBackground, useBGImage);
    defaultBGColor=config->readColorEntry
                   (ConfigView_DefaultBackgroundColor, &white);
    bgImage=config->readEntry
            (ConfigView_DefaultBackgroundImage,
             "konqueror/tiles/kenwimer.png");
    if(useDefaultBGImage)
    {
        unsigned count=0;
        QStringList dirs=KGlobal::instance()->dirs()->findDirs("data", "/");
        if(!dirs.isEmpty())
        {
            for(count=0; count<dirs.count(); ++count)
            {
                // kdDebug() << "Trying " << dirs[count] + "/" + bgImage << endl;
                if(getBackground(dirs[count] + "/" + bgImage, defaultBGImage))
                {
                    break;
                }
            }
        }
        if(count==dirs.count())
        {
            useDefaultBGImage=getBackground(bgImage, defaultBGImage);
            if(!useDefaultBGImage)
            {
                kdDebug(5720) << "KABDetailedView::configure: "
                          << "default BG image selected, but could not be loaded."
                          << endl;
            }
        }
    }
//     kdDebug() << "KABDetailedView::configure: "
//               << "default BG color is " << defaultBGColor.name()
//               << ", default BG image is " << bgImage
//               << ", BG image is "
//               << (useDefaultBGImage ? "in use." : "not used.")
//               << endl;
    // ----- default background color:
    defaultBGColor=config->readColorEntry
                   (ConfigView_DefaultBackgroundColor, &white);
    // -----
    headlineBGColor=config->readColorEntry
                    (ConfigView_HeadlineBGColor, &darkBlue);
    headlineTextColor=config->readColorEntry
                      (ConfigView_HeadlineTextColor, &yellow);
    useHeadlineBGColor=config->readBoolEntry(ConfigView_UseHeadlineBGColor, true);
    // -----
    if(epainter!=0)
    {
        delete epainter;
        epainter=0;
    }
    epainter=new KABEntryPainter
             (Qt::black, headlineTextColor,
              useHeadlineBGColor, headlineBGColor,
              QFont(gfont, gpointsize+4, QFont::Bold, true),
              QFont(gfont, gpointsize+2, QFont::Bold, true),
              QFont(gfont, gpointsize, QFont::Normal, false),
              QFont(ffont, fpointsize, QFont::Normal, false),
              QFont(gfont, gpointsize, QFont::Normal, false));
}

#include "look_details.moc"
