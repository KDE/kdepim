/*
  Copyright (c) 2007 Volker Krause <vkrause@kde.org>
  Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Author: Sergio Martins <sergio.martins@kdab.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef EVENTVIEWS_MULTIAGENDAVIEW_H_H
#define EVENTVIEWS_MULTIAGENDAVIEW_H_H

#include "eventview.h"

#include <QDateTime>

namespace EventViews
{

class ConfigDialogInterface;

/**
  Shows one agenda for every resource side-by-side.
*/
class EVENTVIEWS_EXPORT MultiAgendaView : public EventView
{
    Q_OBJECT
public:
    explicit MultiAgendaView(QWidget *parent = 0);
    ~MultiAgendaView();

    Akonadi::Item::List selectedIncidences() const;
    KCalCore::DateList selectedIncidenceDates() const;
    int currentDateCount() const;
    int maxDatesHint() const;

    bool eventDurationHint(QDateTime &startDt, QDateTime &endDt, bool &allDay) const;

    /* reimp */
    void setCalendar(const Akonadi::ETMCalendar::Ptr &cal) Q_DECL_OVERRIDE;

    /* reimp */
    bool hasConfigurationDialog() const Q_DECL_OVERRIDE;

    void setChanges(Changes changes) Q_DECL_OVERRIDE;

    bool customColumnSetupUsed() const;
    int customNumberOfColumns() const;
    QVector<QString> customColumnTitles() const;
    QVector<KCheckableProxyModel *>collectionSelectionModels() const;

    void setPreferences(const PrefsPtr &prefs);

Q_SIGNALS:
    void showNewEventPopupSignal();
    void showIncidencePopupSignal(const Akonadi::Item &, const QDate &);

public slots:

    void customCollectionsChanged(ConfigDialogInterface *dlg);

    void showDates(const QDate &start, const QDate &end, const QDate &preferredMonth = QDate());
    void showIncidences(const Akonadi::Item::List &incidenceList, const QDate &date);
    void updateView();
    void updateConfig();

    void setIncidenceChanger(Akonadi::IncidenceChanger *changer);

protected:
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;

    /* reimp */void doRestoreConfig(const KConfigGroup &configGroup) Q_DECL_OVERRIDE;
    /* reimp */void doSaveConfig(KConfigGroup &configGroup) Q_DECL_OVERRIDE;

protected Q_SLOTS:
    /**
     * Reimplemented from KOrg::BaseView
     */
    void collectionSelectionChanged();

private slots:
    void slotSelectionChanged();
    void slotClearTimeSpanSelection();
    void resizeSplitters();
    void setupScrollBar();
    void zoomView(const int delta, const QPoint &pos, const Qt::Orientation ori);
    void slotResizeScrollView();
    void recreateViews();
    void forceRecreateViews();

private:
    class Private;
    Private *const d;
};

}

#endif
