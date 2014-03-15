/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "adblockutil.h"

QMap<QString, QString> MessageViewer::AdBlockUtil::listSubscriptions()
{
    QMap<QString, QString> lst;
    lst.insert(QString::fromUtf8("EasyList"), QLatin1String("https://easylist-downloads.adblockplus.org/easylist.txt"));
    lst.insert(QString::fromUtf8("EasyList without element hiding"), QLatin1String("https://easylist-downloads.adblockplus.org/easylist_noelemhide.txt"));
    lst.insert(QString::fromUtf8("Corset"), QLatin1String("http://brianyi.com/corset.txt"));
    lst.insert(QString::fromUtf8("EasyList Germany+EasyList"), QLatin1String("https://easylist-downloads.adblockplus.org/ares+easylist.txt"));
    lst.insert(QString::fromUtf8("EasyList Germany"), QLatin1String("https://easylist-downloads.adblockplus.org/easylistgermany.txt"));
    lst.insert(QString::fromUtf8("Liste FR+EasyList"), QLatin1String("https://easylist-downloads.adblockplus.org/liste_fr+easylist.txt"));
    lst.insert(QString::fromUtf8("Liste FR"), QLatin1String("http://lian.info.tm/liste_fr.txt"));
    lst.insert(QString::fromUtf8("ROList+EasyList"), QLatin1String("https://easylist-downloads.adblockplus.org/rolist+easylist.txt"));
    lst.insert(QString::fromUtf8("ROList"), QLatin1String("http://www.picpoc.ro/menetzrolist.txt"));
    lst.insert(QString::fromUtf8("Việt Nam List"), QLatin1String("http://adblockplus-vietnam.googlecode.com/svn/trunk/abpvn.txt"));
    lst.insert(QString::fromUtf8("AdblockList.org"), QLatin1String("http://adblocklist.org/adblock-pxf-polish.txt"));
    lst.insert(QString::fromUtf8("Bulgarian list"), QLatin1String("http://stanev.org/abp/adblock_bg.txt"));
    lst.insert(QString::fromUtf8("EasyPrivacy+EasyList"), QLatin1String("https://easylist-downloads.adblockplus.org/easyprivacy+easylist.txt"));
    lst.insert(QString::fromUtf8("EasyPrivacy+Cédrics Liste"), QLatin1String("https://easylist-downloads.adblockplus.org/easyprivacy+cedrics.txt"));
    lst.insert(QString::fromUtf8("EasyPrivacy"), QLatin1String("https://easylist-downloads.adblockplus.org/easyprivacy.txt"));
    lst.insert(QString::fromUtf8("void.gr"), QLatin1String("http://www.void.gr/kargig/void-gr-filters.txt"));
    lst.insert(QString::fromUtf8("Wiltteri"), QLatin1String("http://www.wiltteri.net/wiltteri.txt"));
    lst.insert(QString::fromUtf8("ChinaList"), QLatin1String("http://adblock-chinalist.googlecode.com/svn/trunk/adblock.txt"));
    lst.insert(QString::fromUtf8("Filter von Dr.Evil"), QLatin1String("http://adblock.maltekraus.de/adblock.txt"));
    lst.insert(QString::fromUtf8("RuAdList"), QLatin1String("http://ruadlist.googlecode.com/svn/trunk/adblock.txt"));
    //lst.insert(QString::fromUtf8("AdblockRules.org"), QLatin1String("http://adblockrules.org/download.php?typeall"));
    lst.insert(QString::fromUtf8("BSI Lista Polska"), QLatin1String("http://www.bsi.info.pl/filtrABP.txt"));
    lst.insert(QString::fromUtf8("Czech List"), QLatin1String("http://dajbych.net/adblock.txt"));
    lst.insert(QString::fromUtf8("Cédrics Liste"), QLatin1String("http://chewey.de/mozilla/data/adblock.txt"));
    lst.insert(QString::fromUtf8("Fanboy's List"), QLatin1String("http://www.fanboy.co.nz/adblock/fanboy-adblocklist-current-expanded.txt"));
    lst.insert(QString::fromUtf8("Filter von MonztA"), QLatin1String("http://monzta.maltekraus.de/adblock.txt"));
    lst.insert(QString::fromUtf8("hufilter"), QLatin1String("http://pete.teamlupus.hu/hufilter.txt"));
    lst.insert(QString::fromUtf8("Iceland List"), QLatin1String("http://adblock-iceland.googlecode.com/files/icelandic%20filter.txt"));
    lst.insert(QString::fromUtf8("Japanese General Filter"), QLatin1String("http://adblock-plus-japanese-filter.googlecode.com/svn/trunk/abp_jp_general.txt"));
    lst.insert(QString::fromUtf8("Japanese Site-Specific Filter"), QLatin1String("http://adblock-plus-japanese-filter.googlecode.com/svn/trunk/abp_jp_site_specific.txt"));
    lst.insert(QString::fromUtf8("NLBlock"), QLatin1String("http://www.verzijlbergh.com/adblock/nlblock.txt"));
    lst.insert(QString::fromUtf8("PLgeneral"), QLatin1String("http://www.niecko.pl/adblock/adblock.txt"));
    lst.insert(QString::fromUtf8("Schacks Adblock Plus liste"), QLatin1String("http://adblock.schack.dk/block.txt"));
    lst.insert(QString::fromUtf8("Xfiles"), QLatin1String("http://mozilla.gfsolone.com/filtri.txt"));
    lst.insert(QString::fromUtf8("adblock.free.fr"), QLatin1String("http://adblock.free.fr/adblock.txt"));
    lst.insert(QString::fromUtf8("adblock.free.fr basic (bloque les pubs uniquement)"), QLatin1String("http://adblock.free.fr/adblock_basic.txt"));
    lst.insert(QString::fromUtf8("Ajnasz's list"), QLatin1String("http://ajnasz.hu/adblock/recent"));
    lst.insert(QString::fromUtf8("Schuzak's Universal Filter"), QLatin1String("http://www.schuzak.jp/other/abp.txt"));
    lst.insert(QString::fromUtf8("Rickroll Blacklist"), QLatin1String("http://rickrolldb.com/ricklist.txt"));
    lst.insert(QString::fromUtf8("Corset+EasyList"), QLatin1String("https://easylist-downloads.adblockplus.org/corset+easylist.txt"));
    return lst;
}
