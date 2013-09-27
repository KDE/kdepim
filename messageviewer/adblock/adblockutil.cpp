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
    lst.insert(QLatin1String("EasyList"), QLatin1String("https://easylist-downloads.adblockplus.org/easylist.txt"));
    lst.insert(QLatin1String("EasyList without element hiding"), QLatin1String("https://easylist-downloads.adblockplus.org/easylist_noelemhide.txt"));
    lst.insert(QLatin1String("Corset"), QLatin1String("http://brianyi.com/corset.txt"));
    lst.insert(QLatin1String("EasyList Germany+EasyList"), QLatin1String("https://easylist-downloads.adblockplus.org/ares+easylist.txt"));
    lst.insert(QLatin1String("EasyList Germany"), QLatin1String("https://easylist-downloads.adblockplus.org/easylistgermany.txt"));
    lst.insert(QLatin1String("Liste FR+EasyList"), QLatin1String("https://easylist-downloads.adblockplus.org/liste_fr+easylist.txt"));
    lst.insert(QLatin1String("Liste FR"), QLatin1String("http://lian.info.tm/liste_fr.txt"));
    lst.insert(QLatin1String("ROList+EasyList"), QLatin1String("https://easylist-downloads.adblockplus.org/rolist+easylist.txt"));
    lst.insert(QLatin1String("ROList"), QLatin1String("http://www.picpoc.ro/menetzrolist.txt"));
    lst.insert(QLatin1String("Việt Nam List+EasyList"), QLatin1String("https://easylist-downloads.adblockplus.org/abpvn+easylist.txt"));
    lst.insert(QLatin1String("Việt Nam List"), QLatin1String("http://adblockplus-vietnam.googlecode.com/svn/trunk/abpvn.txt"));
    lst.insert(QLatin1String("AdblockList.org"), QLatin1String("http://adblocklist.org/adblock-pxf-polish.txt"));

    lst.insert(QLatin1String("Bulgarian list"), QLatin1String("http://stanev.org/abp/adblock_bg.txt"));

    lst.insert(QLatin1String("EasyPrivacy+EasyList"), QLatin1String("https://easylist-downloads.adblockplus.org/easyprivacy+easylist.txt"));

    lst.insert(QLatin1String("EasyPrivacy+Cédrics Liste"), QLatin1String("https://easylist-downloads.adblockplus.org/easyprivacy+cedrics.txt"));

    lst.insert(QLatin1String("EasyPrivacy"), QLatin1String("https://easylist-downloads.adblockplus.org/easyprivacy.txt"));

    lst.insert(QLatin1String("IsraelList"), QLatin1String("http://israellist.googlecode.com/files/IsraelList.txt"));

    lst.insert(QLatin1String("Morpeh Rus List+EasyList"), QLatin1String("https://easylist-downloads.adblockplus.org/morpeh+easylist.txt"));

    lst.insert(QLatin1String("Morpeh Rus List"), QLatin1String("http://adblockplus.mihalkin.ru/Russia.txt"));

    lst.insert(QLatin1String("Norsk adblockliste"), QLatin1String("http://home.online.no/~mlangsho/adblock.txt"));

    lst.insert(QLatin1String("void.gr"), QLatin1String("http://www.void.gr/kargig/void-gr-filters.txt"));

    lst.insert(QLatin1String("Wiltteri"), QLatin1String("http://www.wiltteri.net/wiltteri.txt"));

    lst.insert(QLatin1String("ChinaList"), QLatin1String("http://adblock-chinalist.googlecode.com/svn/trunk/adblock.txt"));

    lst.insert(QLatin1String("Filter von Dr.Evil"), QLatin1String("http://adblock.maltekraus.de/adblock.txt"));

    lst.insert(QLatin1String("RuAdList"), QLatin1String("http://ruadlist.googlecode.com/svn/trunk/adblock.txt"));

    lst.insert(QLatin1String("AdblockRules.org"), QLatin1String("http://adblockrules.org/download.php?typeall"));

    lst.insert(QLatin1String("BSI Lista Polska"), QLatin1String("http://www.bsi.info.pl/filtrABP.txt"));

    lst.insert(QLatin1String("Czech List"), QLatin1String("http://dajbych.net/adblock.txt"));

    lst.insert(QLatin1String("Cédrics Liste"), QLatin1String("http://chewey.de/mozilla/data/adblock.txt"));

    lst.insert(QLatin1String("dutchblock"), QLatin1String("http://dutchmega.nl/dutchblock/list.txt"));

    lst.insert(QLatin1String("Fanboy's List"), QLatin1String("http://www.fanboy.co.nz/adblock/fanboy-adblocklist-current-expanded.txt"));

    lst.insert(  QLatin1String("Fanboy's Chinese"), QLatin1String("http://www.fanboy.co.nz/adblock/fanboy-adblocklist-chn.txt"));

    lst.insert(  QLatin1String("Fanboy's Czech"), QLatin1String("http://www.fanboy.co.nz/adblock/fanboy-adblocklist-cz.txt"));

    lst.insert(  QLatin1String("Fanboy's Espanol/Portuguese"), QLatin1String("http://www.fanboy.co.nz/adblock/fanboy-adblocklist-esp.txt"));

    lst.insert(  QLatin1String("Fanboy's Japanese"), QLatin1String("http://www.fanboy.co.nz/adblock/fanboy-adblocklist-jpn.txt"));

    lst.insert(  QLatin1String("Fanboy's Korean"), QLatin1String("http://www.fanboy.co.nz/adblock/fanboy-adblocklist-krn.txt"));

    lst.insert(  QLatin1String("Fanboy's Tracking/Stats Blocking"), QLatin1String("http://www.fanboy.co.nz/adblock/fanboy-adblocklist-stats.txt"));

    lst.insert(  QLatin1String("Fanboy's Turkish"), QLatin1String("http://www.fanboy.co.nz/adblock/fanboy-adblocklist-tky.txt"));

    lst.insert(  QLatin1String("Filter von MonztA"), QLatin1String("http://monzta.maltekraus.de/adblock.txt"));

    lst.insert(  QLatin1String("Filtros Nauscopicos"), QLatin1String("http://s3.amazonaws.com/lcp/maty/myfiles/AdBlock-Nauscopio-maty.txt"));

    lst.insert(  QLatin1String("Hackrus anti-advertisement"), QLatin1String("http://nsis.narod.ru/clean_internet/aag.txt"));

    lst.insert(  QLatin1String("Hackrus anti-advertisement supplemental"), QLatin1String("http://nsis.narod.ru/clean_internet/aas.txt"));

    lst.insert(  QLatin1String("Hackrus anti-counter"), QLatin1String("http://nsis.narod.ru/clean_internet/ac.txt"));

    lst.insert(  QLatin1String("hufilter"), QLatin1String("http://pete.teamlupus.hu/hufilter.txt"));

    lst.insert(  QLatin1String("Iceland List"), QLatin1String("http://adblock-iceland.googlecode.com/files/icelandic%20filter.txt"));

    lst.insert(  QLatin1String("Japanese General Filter"), QLatin1String("http://adblock-plus-japanese-filter.googlecode.com/svn/trunk/abp_jp_general.txt"));

    lst.insert(  QLatin1String("Japanese Site-Specific Filter"), QLatin1String("http://adblock-plus-japanese-filter.googlecode.com/svn/trunk/abp_jp_site_specific.txt"));

    lst.insert(  QLatin1String("Lista Basa"), QLatin1String("http://www.photographer.neostrada.pl/abp.txt"));

    lst.insert(  QLatin1String("NLBlock"), QLatin1String("http://www.verzijlbergh.com/adblock/nlblock.txt"));

    lst.insert(  QLatin1String("PLgeneral"), QLatin1String("http://www.niecko.pl/adblock/adblock.txt"));

    lst.insert(  QLatin1String("Schacks Adblock Plus liste"), QLatin1String("http://adblock.schack.dk/block.txt"));

    lst.insert(  QLatin1String("UA-IX Бан-лист"), QLatin1String("http://adblock.oasis.org.ua/banlist.txt"));

    lst.insert(  QLatin1String("Xfiles"), QLatin1String("http://mozilla.gfsolone.com/filtri.txt"));

    lst.insert(  QLatin1String("adblock.free.fr"), QLatin1String("http://adblock.free.fr/adblock.txt"));

    lst.insert(  QLatin1String("adblock.free.fr basic (bloque les pubs uniquement)"), QLatin1String("http://adblock.free.fr/adblock_basic.txt"));

    lst.insert(  QLatin1String("Ajnasz's list"), QLatin1String("http://ajnasz.hu/adblock/recent"));

    lst.insert(  QLatin1String("Schuzak's Universal Filter"), QLatin1String("http://www.schuzak.jp/other/abp.txt"));

    lst.insert(  QLatin1String("Malware Domains"),QLatin1String("http://malwaredomains.lanik.us/malwaredomains_full.txt"));

    lst.insert(  QLatin1String("Rickroll Blacklist"), QLatin1String("http://rickrolldb.com/ricklist.txt"));

    lst.insert(  QLatin1String("Corset+EasyList"), QLatin1String("https://easylist-downloads.adblockplus.org/corset+easylist.txt"));
    return lst;
}
