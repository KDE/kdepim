/***************************************************************************
 *   Copyright 2009 by Sebastian Kügler <sebas@kde.org>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

//own
#include "kpapplet.h"
#include "kpdialog.h"

//Qt
#include <QTimer>
#include <QClipboard>

//KDE
#include <KIcon>
#include <KConfigDialog>
#include <KStandardDirs>
#include <KToolInvocation>

//plasma
#include <Plasma/Dialog>
//use for desktop view
#include <Plasma/IconWidget>
#include <Plasma/Theme>
#include <Plasma/ToolTipManager>


using namespace Plasma;
using namespace KP;

K_EXPORT_PLASMA_APPLET(kpapplet, KPApplet)

KPApplet::KPApplet(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args),
      m_icon(0),
      m_dialog(0)
{


    setBackgroundHints(StandardBackground);
    setAspectRatioMode(IgnoreAspectRatio);
    setHasConfigurationInterface(true);
    setAcceptsHoverEvents(true);

    // initialize the widget
    (void)widget();
}

KPApplet::~KPApplet()
{
    delete m_icon;
    delete m_dialog;
}

void KPApplet::init()
{
    //KConfigGroup cg = config();

    m_icon = new Plasma::IconWidget(KIcon(QLatin1String("kmail"),NULL), QString());

    Plasma::ToolTipManager::self()->registerWidget(this);

    setPopupIcon(m_icon->icon());

    updateToolTip(1337);
}

QGraphicsWidget* KPApplet::graphicsWidget()
{
    if (!m_dialog) {
        m_dialog = new KPDialog(this);
    }
    return m_dialog->dialog();
}

void KPApplet::popupEvent(bool show)
{
    if (show) {
        qDebug() << "showing";
    }
}

void KPApplet::updateToolTip(const int emails)
{

    m_toolTip = Plasma::ToolTipContent(i18nc("Tooltip main title text", "Your emails"),
                        i18ncp("Tooltip sub text", "One new email", "%1 new emails", emails),
                        KIcon(QLatin1String("kmail")).pixmap(IconSize(KIconLoader::Desktop))
                    );
    Plasma::ToolTipManager::self()->setContent(this, m_toolTip);
}

