/*
  Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Copyright (C) 2012  Allen Winter <winter@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

#include "incidencedialog.h"
#include "incidenceeditor_debug.h"
#include "combinedincidenceeditor.h"
#include "editorconfig.h"
#include "incidencealarm.h"
#include "incidenceattachment.h"
#include "incidenceattendee.h"
#include "incidencecategories.h"
#include "incidencecompletionpriority.h"
#include "incidencedatetime.h"
#include "incidencedescription.h"
#include "incidencerecurrence.h"
#include "incidenceresource.h"
#include "incidencesecrecy.h"
#include "incidencewhatwhere.h"
#include "templatemanagementdialog.h"
#include "ui_dialogdesktop.h"

#include <incidenceeditorsettings.h>

#include <CalendarSupport/KCalPrefs>
#include <CalendarSupport/Utils>

#include <CollectionComboBox>
#include <Item>
#include <Akonadi/Calendar/ETMCalendar>

#include <KCalCore/ICalFormat>
#include <KCalCore/MemoryCalendar>
#include <KCalUtils/Stringify>

#include <KMessageBox>
#include <KSystemTimeZones>
#include <KIconLoader>
#include <KSharedConfig>

#include <QCloseEvent>
#include <QDir>
#include <QIcon>
#include <QStandardPaths>

using namespace IncidenceEditorNG;

namespace IncidenceEditorNG
{

enum Tabs {
    GeneralTab = 0,
    AttendeesTab,
    ResourcesTab,
    AlarmsTab,
    RecurrenceTab,
    AttachmentsTab
};

class IncidenceDialogPrivate : public ItemEditorUi
{
    IncidenceDialog *q_ptr;
    Q_DECLARE_PUBLIC(IncidenceDialog)

public:
    Ui::EventOrTodoDesktop *mUi;
    Akonadi::CollectionComboBox *mCalSelector;
    bool mCloseOnSave;

    EditorItemManager *mItemManager;
    CombinedIncidenceEditor *mEditor;
    IncidenceDateTime *mIeDateTime;
    IncidenceAttendee *mIeAttendee;
    IncidenceRecurrence *mIeRecurrence;
    IncidenceResource *mIeResource;
    bool mInitiallyDirty;
    Akonadi::Item mItem;
    QString typeToString(const int type) const;

public:
    IncidenceDialogPrivate(Akonadi::IncidenceChanger *changer, IncidenceDialog *qq);
    ~IncidenceDialogPrivate();

    /// General methods
    void handleAlarmCountChange(int newCount);
    void handleRecurrenceChange(IncidenceEditorNG::RecurrenceType type);
    void loadTemplate(const QString &templateName);
    void manageTemplates();
    void saveTemplate(const QString &templateName);
    void storeTemplatesInConfig(const QStringList &newTemplates);
    void updateAttachmentCount(int newCount);
    void updateAttendeeCount(int newCount);
    void updateResourceCount(int newCount);
    void updateButtonStatus(bool isDirty);
    void showMessage(const QString &text, KMessageWidget::MessageType type);
    void slotInvalidCollection();

    /// ItemEditorUi methods
    bool containsPayloadIdentifiers(const QSet<QByteArray> &partIdentifiers) const Q_DECL_OVERRIDE;
    void handleItemSaveFinish(EditorItemManager::SaveAction);
    void handleItemSaveFail(EditorItemManager::SaveAction, const QString &errorMessage);
    bool hasSupportedPayload(const Akonadi::Item &item) const Q_DECL_OVERRIDE;
    bool isDirty() const Q_DECL_OVERRIDE;
    bool isValid() const Q_DECL_OVERRIDE;
    void load(const Akonadi::Item &item) Q_DECL_OVERRIDE;
    Akonadi::Item save(const Akonadi::Item &item) Q_DECL_OVERRIDE;
    Akonadi::Collection selectedCollection() const Q_DECL_OVERRIDE;
    void slotButtonClicked(int button);

    void reject(RejectReason reason, const QString &errorMessage = QString()) Q_DECL_OVERRIDE;
};

}

IncidenceDialogPrivate::IncidenceDialogPrivate(Akonadi::IncidenceChanger *changer,
        IncidenceDialog *qq)
    : q_ptr(qq),
      mUi(new Ui::EventOrTodoDesktop),
      mCalSelector(new Akonadi::CollectionComboBox),
      mCloseOnSave(false),
      mItemManager(new EditorItemManager(this, changer)),
      mEditor(new CombinedIncidenceEditor(qq)),
      mInitiallyDirty(false)
{
    Q_Q(IncidenceDialog);
    mUi->setupUi(q->mainWidget());
    QGridLayout *layout = new QGridLayout(mUi->mCalSelectorPlaceHolder);
    layout->setSpacing(0);
    layout->addWidget(mCalSelector);
    mCalSelector->setAccessRightsFilter(Akonadi::Collection::CanCreateItem);

    q->connect(mCalSelector, &Akonadi::CollectionComboBox::currentChanged, q, &IncidenceDialog::handleSelectedCollectionChange);

    // Now instantiate the logic of the dialog. These editors update the ui, validate
    // fields and load/store incidences in the ui.
    IncidenceWhatWhere *ieGeneral = new IncidenceWhatWhere(mUi);
    mEditor->combine(ieGeneral);

    IncidenceCategories *ieCategories = new IncidenceCategories(mUi);
    mEditor->combine(ieCategories);

    mIeDateTime = new IncidenceDateTime(mUi);
    mEditor->combine(mIeDateTime);

    IncidenceCompletionPriority *ieCompletionPriority = new IncidenceCompletionPriority(mUi);
    mEditor->combine(ieCompletionPriority);

    IncidenceDescription *ieDescription = new IncidenceDescription(mUi);
    mEditor->combine(ieDescription);

    IncidenceAlarm *ieAlarm = new IncidenceAlarm(mIeDateTime, mUi);
    mEditor->combine(ieAlarm);

    IncidenceAttachment *ieAttachments = new IncidenceAttachment(mUi);
    mEditor->combine(ieAttachments);

    mIeRecurrence = new IncidenceRecurrence(mIeDateTime, mUi);
    mEditor->combine(mIeRecurrence);

    IncidenceSecrecy *ieSecrecy = new IncidenceSecrecy(mUi);
    mEditor->combine(ieSecrecy);

    mIeAttendee = new IncidenceAttendee(qq, mIeDateTime, mUi);
    mIeAttendee->setParent(qq);
    mEditor->combine(mIeAttendee);

    mIeResource = new IncidenceResource(mIeAttendee, mIeDateTime, mUi);
    mEditor->combine(mIeResource);

    q->connect(mEditor, SIGNAL(showMessage(QString,KMessageWidget::MessageType)),
               SLOT(showMessage(QString,KMessageWidget::MessageType)));
    q->connect(mEditor, SIGNAL(dirtyStatusChanged(bool)),
               SLOT(updateButtonStatus(bool)));
    q->connect(mItemManager, SIGNAL(itemSaveFinished(IncidenceEditorNG::EditorItemManager::SaveAction)),
               SLOT(handleItemSaveFinish(IncidenceEditorNG::EditorItemManager::SaveAction)));
    q->connect(mItemManager, SIGNAL(itemSaveFailed(IncidenceEditorNG::EditorItemManager::SaveAction,QString)),
               SLOT(handleItemSaveFail(IncidenceEditorNG::EditorItemManager::SaveAction,QString)));
    q->connect(ieAlarm, SIGNAL(alarmCountChanged(int)),
               SLOT(handleAlarmCountChange(int)));
    q->connect(mIeRecurrence, SIGNAL(recurrenceChanged(IncidenceEditorNG::RecurrenceType)),
               SLOT(handleRecurrenceChange(IncidenceEditorNG::RecurrenceType)));
    q->connect(ieAttachments, SIGNAL(attachmentCountChanged(int)),
               SLOT(updateAttachmentCount(int)));
    q->connect(mIeAttendee, SIGNAL(attendeeCountChanged(int)),
               SLOT(updateAttendeeCount(int)));
    q->connect(mIeResource, SIGNAL(resourceCountChanged(int)),
               SLOT(updateResourceCount(int)));
}

IncidenceDialogPrivate::~IncidenceDialogPrivate()
{
    delete mItemManager;
    delete mEditor;
    delete mUi;
}

void IncidenceDialogPrivate::slotInvalidCollection()
{
    showMessage(i18n("Select a valid collection first."), KMessageWidget::Warning);
}

void IncidenceDialogPrivate::showMessage(const QString &text, KMessageWidget::MessageType type)
{
    mUi->mMessageWidget->setText(text);
    mUi->mMessageWidget->setMessageType(type);
    mUi->mMessageWidget->show();
}

void IncidenceDialogPrivate::handleAlarmCountChange(int newCount)
{
    QString tabText;
    if (newCount > 0) {
        tabText =
            i18nc("@title:tab Tab to configure the reminders of an event or todo",
                  "Reminder (%1)", newCount);
    } else {
        tabText =
            i18nc("@title:tab Tab to configure the reminders of an event or todo",
                  "Reminder");
    }

    mUi->mTabWidget->setTabText(AlarmsTab, tabText);
}

void IncidenceDialogPrivate::handleRecurrenceChange(IncidenceEditorNG::RecurrenceType type)
{
    QString tabText =
        i18nc("@title:tab Tab to configure the recurrence of an event or todo",
              "Rec&urrence");

    // Keep this numbers in sync with the items in mUi->mRecurrenceTypeCombo. I
    // tried adding an enum to IncidenceRecurrence but for whatever reason I could
    // Qt not play nice with namespaced enums in signal/slot connections.
    // Anyways, I don't expect these values to change.
    switch (type) {
    case RecurrenceTypeNone:
        break;
    case RecurrenceTypeDaily:
        tabText += i18nc("@title:tab Daily recurring event, capital first letter only", " (D)");
        break;
    case RecurrenceTypeWeekly:
        tabText += i18nc("@title:tab Weekly recurring event, capital first letter only", " (W)");
        break;
    case RecurrenceTypeMonthly:
        tabText += i18nc("@title:tab Monthly recurring event, capital first letter only", " (M)");
        break;
    case RecurrenceTypeYearly:
        tabText += i18nc("@title:tab Yearly recurring event, capital first letter only", " (Y)");
        break;
    case RecurrenceTypeException:
        tabText += i18nc("@title:tab Exception to a recurring event, capital first letter only", " (E)");
        break;
    default:
        Q_ASSERT_X(false, "handleRecurrenceChange", "Fix your program");
    }

    mUi->mTabWidget->setTabText(RecurrenceTab, tabText);
}

QString IncidenceDialogPrivate::typeToString(const int type) const
{
    // Do not translate.
    switch (type) {
    case KCalCore::Incidence::TypeEvent:
        return QStringLiteral("Event");
    case KCalCore::Incidence::TypeTodo:
        return QStringLiteral("Todo");
    case KCalCore::Incidence::TypeJournal:
        return QStringLiteral("Journal");
    default:
        return QStringLiteral("Unknown");
    }
}

void IncidenceDialogPrivate::loadTemplate(const QString &templateName)
{
    Q_Q(IncidenceDialog);

    KCalCore::MemoryCalendar::Ptr cal(new KCalCore::MemoryCalendar(KSystemTimeZones::local()));

    const QString fileName = QStandardPaths::locate(
                                 QStandardPaths::GenericDataLocation,
                                 "korganizer/templates/" +
                                 typeToString(mEditor->type()) + '/' +
                                 templateName);

    if (fileName.isEmpty()) {
        KMessageBox::error(
            q,
            i18nc("@info", "Unable to find template '%1'.", fileName));
        return;
    }

    KCalCore::ICalFormat format;
    if (!format.load(cal, fileName)) {
        KMessageBox::error(
            q,
            i18nc("@info", "Error loading template file '%1'.", fileName));
        return;
    }

    KCalCore::Incidence::List incidences = cal->incidences();
    if (incidences.isEmpty()) {
        KMessageBox::error(
            q,
            i18nc("@info", "Template does not contain a valid incidence."));
        return;
    }

    mIeDateTime->setActiveDate(QDate());
    KCalCore::Incidence::Ptr newInc = KCalCore::Incidence::Ptr(incidences.first()->clone());
    newInc->setUid(KCalCore::CalFormat::createUniqueId());

    // We add a custom property so that some fields aren't loaded, dates for example
    newInc->setCustomProperty(QByteArray("kdepim"), "isTemplate", QStringLiteral("true"));
    mEditor->load(newInc);
    newInc->removeCustomProperty(QByteArray(), "isTemplate");
}

void IncidenceDialogPrivate::manageTemplates()
{
    Q_Q(IncidenceDialog);

    QStringList &templates =
        IncidenceEditorNG::EditorConfig::instance()->templates(mEditor->type());

    QPointer<IncidenceEditorNG::TemplateManagementDialog> dialog(
        new IncidenceEditorNG::TemplateManagementDialog(
            q, templates, KCalUtils::Stringify::incidenceType(mEditor->type())));

    q->connect(dialog, SIGNAL(loadTemplate(QString)),
               SLOT(loadTemplate(QString)));
    q->connect(dialog, SIGNAL(templatesChanged(QStringList)),
               SLOT(storeTemplatesInConfig(QStringList)));
    q->connect(dialog, SIGNAL(saveTemplate(QString)),
               SLOT(saveTemplate(QString)));
    dialog->exec();
    delete dialog;
}

void IncidenceDialogPrivate::saveTemplate(const QString &templateName)
{
    Q_ASSERT(! templateName.isEmpty());

    KCalCore::MemoryCalendar::Ptr cal(new KCalCore::MemoryCalendar(KSystemTimeZones::local()));

    switch (mEditor->type()) {
    case KCalCore::Incidence::TypeEvent: {
        KCalCore::Event::Ptr event(new KCalCore::Event());
        mEditor->save(event);
        cal->addEvent(KCalCore::Event::Ptr(event->clone()));
        break;
    }
    case KCalCore::Incidence::TypeTodo: {
        KCalCore::Todo::Ptr todo(new KCalCore::Todo);
        mEditor->save(todo);
        cal->addTodo(KCalCore::Todo::Ptr(todo->clone()));
        break;
    }
    case KCalCore::Incidence::TypeJournal: {
        KCalCore::Journal::Ptr journal(new KCalCore::Journal);
        mEditor->save(journal);
        cal->addJournal(KCalCore::Journal::Ptr(journal->clone()));
        break;
    }
    default:
        Q_ASSERT_X(false, "saveTemplate", "Fix your program");
    }

    QString fileName = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) +
                       "korganizer/templates/" + typeToString(mEditor->type()) + '/';
    QDir::root().mkpath(fileName);
    fileName += templateName;

    KCalCore::ICalFormat format;
    format.save(cal, fileName);
}

void IncidenceDialogPrivate::storeTemplatesInConfig(const QStringList &templateNames)
{
    // I find this somewhat broken. templates() returns a reference, maybe it should
    // be changed by adding a setTemplates method.
    const QStringList origTemplates =
        IncidenceEditorNG::EditorConfig::instance()->templates(mEditor->type());
    const QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) +
                                "korganizer/templates/" +
                                typeToString(mEditor->type()) + '/';
    QDir::root().mkpath(defaultPath);
    Q_FOREACH (const QString &tmpl, origTemplates) {
        if (!templateNames.contains(tmpl)) {
            const QString fileName = defaultPath + tmpl;
            QFile file(fileName);
            if (file.exists()) {
                file.remove();
            }
        }
    }

    IncidenceEditorNG::EditorConfig::instance()->templates(mEditor->type()) = templateNames;
    IncidenceEditorNG::EditorConfig::instance()->config()->save();
}

void IncidenceDialogPrivate::updateAttachmentCount(int newCount)
{
    if (newCount > 0) {
        mUi->mTabWidget->setTabText(
            AttachmentsTab,
            i18nc("@title:tab Tab to modify attachments of an event or todo",
                  "Attac&hments (%1)", newCount));
    } else {
        mUi->mTabWidget->setTabText(
            AttachmentsTab,
            i18nc("@title:tab Tab to modify attachments of an event or todo",
                  "Attac&hments"));
    }
}

void IncidenceDialogPrivate::updateAttendeeCount(int newCount)
{
    if (newCount > 0) {
        mUi->mTabWidget->setTabText(
            AttendeesTab,
            i18nc("@title:tab Tab to modify attendees of an event or todo",
                  "&Attendees (%1)", newCount));
    } else {
        mUi->mTabWidget->setTabText(
            AttendeesTab,
            i18nc("@title:tab Tab to modify attendees of an event or todo",
                  "&Attendees"));
    }
}

void IncidenceDialogPrivate::updateResourceCount(int newCount)
{
    if (newCount > 0) {
        mUi->mTabWidget->setTabText(
            ResourcesTab,
            i18nc("@title:tab Tab to modify attendees of an event or todo",
                  "&Resources (%1)", newCount));
    } else {
        mUi->mTabWidget->setTabText(
            ResourcesTab,
            i18nc("@title:tab Tab to modify attendees of an event or todo",
                  "&Resources"));
    }
}

void IncidenceDialogPrivate::updateButtonStatus(bool isDirty)
{
    Q_Q(IncidenceDialog);
    q->enableButton(KDialog::Apply, isDirty || mInitiallyDirty);
}

bool IncidenceDialogPrivate::containsPayloadIdentifiers(
    const QSet<QByteArray> &partIdentifiers) const
{
    return partIdentifiers.contains(QByteArray("PLD:RFC822"));
}

void IncidenceDialogPrivate::handleItemSaveFail(EditorItemManager::SaveAction,
        const QString &errorMessage)
{
    Q_Q(IncidenceDialog);

    bool retry = false;

    if (!errorMessage.isEmpty()) {
        const QString message = i18nc("@info",
                                      "Unable to store the incidence in the calendar. Try again?\n\n "
                                      "Reason: %1", errorMessage);
        retry = (KMessageBox::warningYesNo(q, message) == KMessageBox::Yes);
    }

    if (retry) {
        mItemManager->save();
    } else {
        updateButtonStatus(isDirty());
        q->enableButtonOk(true);
        q->enableButtonCancel(true);
    }
}

void IncidenceDialogPrivate::handleItemSaveFinish(EditorItemManager::SaveAction saveAction)
{
    Q_Q(IncidenceDialog);

    if (mCloseOnSave) {
        q->accept();
    } else {
        const Akonadi::Item item = mItemManager->item();
        Q_ASSERT(item.isValid());
        Q_ASSERT(item.hasPayload());
        Q_ASSERT(item.hasPayload<KCalCore::Incidence::Ptr>());
        // Now the item is succesfull saved, reload it in the editor in order to
        // reset the dirty status of the editor.
        mEditor->load(item.payload<KCalCore::Incidence::Ptr>());
        mEditor->load(item);

        // Set the buttons to a reasonable state as well (ok and apply should be
        // disabled at this point).
        q->enableButtonOk(true);
        q->enableButtonCancel(true);
        q->enableButtonApply(isDirty());
    }

    if (saveAction == EditorItemManager::Create) {
        Q_EMIT q->incidenceCreated(mItemManager->item());
    }
}

bool IncidenceDialogPrivate::hasSupportedPayload(const Akonadi::Item &item) const
{
    return CalendarSupport::incidence(item);
}

bool IncidenceDialogPrivate::isDirty() const
{
    if (mItem.isValid()) {
        return mEditor->isDirty() ||
               mCalSelector->currentCollection().id() != mItem.storageCollectionId();
    } else {
        return mEditor->isDirty();
    }
}

bool IncidenceDialogPrivate::isValid() const
{
    Q_Q(const IncidenceDialog);
    if (mEditor->isValid()) {
        // Check if there's a selected collection.
        if (mCalSelector->currentCollection().isValid()) {
            return true;
        } else {
            qCWarning(INCIDENCEEDITOR_LOG) << "Select a collection first";
            Q_EMIT q->invalidCollection();
        }
    }

    return false;
}

void IncidenceDialogPrivate::load(const Akonadi::Item &item)
{
    Q_Q(IncidenceDialog);

    Q_ASSERT(hasSupportedPayload(item));

    if (CalendarSupport::hasJournal(item)) {
        //mUi->mTabWidget->removeTab(5);
        mUi->mTabWidget->removeTab(AttachmentsTab);
        mUi->mTabWidget->removeTab(RecurrenceTab);
        mUi->mTabWidget->removeTab(AlarmsTab);
        mUi->mTabWidget->removeTab(AttendeesTab);
        mUi->mTabWidget->removeTab(ResourcesTab);
    }

    mEditor->load(CalendarSupport::incidence(item));
    mEditor->load(item);

    const KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence(item);
    const QStringList allEmails = IncidenceEditorNG::EditorConfig::instance()->allEmails();
    KCalCore::Attendee::Ptr me = incidence->attendeeByMails(allEmails);

    if (incidence->attendeeCount() > 1 &&  // >1 because you won't drink alone
            me && (me->status() == KCalCore::Attendee::NeedsAction ||
                   me->status() == KCalCore::Attendee::Tentative ||
                   me->status() == KCalCore::Attendee::InProcess)) {
        // Show the invitation bar: "You are invited [accept] [decline]"
        mUi->mInvitationBar->show();
    } else {
        mUi->mInvitationBar->hide();
    }

    qCDebug(INCIDENCEEDITOR_LOG) << "Loading item " << item.id() << "; parent " << item.parentCollection().id()
                                 << "; storage " << item.storageCollectionId();

    if (item.storageCollectionId() > -1) {
        mCalSelector->setDefaultCollection(Akonadi::Collection(item.storageCollectionId()));
    }

    if (!mCalSelector->mimeTypeFilter().contains(QStringLiteral("text/calendar")) ||
            !mCalSelector->mimeTypeFilter().contains(incidence->mimeType())) {
        mCalSelector->setMimeTypeFilter(QStringList() << incidence->mimeType() << QStringLiteral("text/calendar"));
    }

    if (mEditor->type() == KCalCore::Incidence::TypeTodo) {
        q->setWindowIcon(QIcon::fromTheme(QStringLiteral("view-calendar-tasks")));
    } else if (mEditor->type() == KCalCore::Incidence::TypeEvent) {
        q->setWindowIcon(QIcon::fromTheme(QStringLiteral("view-calendar-day")));
    } else if (mEditor->type() == KCalCore::Incidence::TypeJournal) {
        q->setWindowIcon(QIcon::fromTheme(QStringLiteral("view-pim-journal")));
    }

    // Initialize tab's titles
    updateAttachmentCount(incidence->attachments().size());
    updateResourceCount(mIeResource->resourceCount());
    updateAttendeeCount(mIeAttendee->attendeeCount());
    handleRecurrenceChange(mIeRecurrence->currentRecurrenceType());
    handleAlarmCountChange(incidence->alarms().count());

    mItem = item;

    q->show();
}

Akonadi::Item IncidenceDialogPrivate::save(const Akonadi::Item &item)
{
    Q_ASSERT(mEditor->incidence<KCalCore::Incidence>());

    KCalCore::Incidence::Ptr incidenceInEditor = mEditor->incidence<KCalCore::Incidence>();
    KCalCore::Incidence::Ptr newIncidence(incidenceInEditor->clone());

    Akonadi::Item result = item;
    result.setMimeType(newIncidence->mimeType());

    // There's no editor that has the relatedTo property. We must set it here, by hand.
    // Otherwise it gets lost.
    // FIXME: Why don't we clone() incidenceInEditor then pass the clone to save(),
    // I wonder if we're not leaking other properties.
    newIncidence->setRelatedTo(incidenceInEditor->relatedTo());

    mEditor->save(newIncidence);
    mEditor->save(result);

    // TODO: Remove this once we support moving of events/todo's
    mCalSelector->setEnabled(false);

    // Make sure that we don't loose uid for existing incidence
    newIncidence->setUid(mEditor->incidence<KCalCore::Incidence>()->uid());

    // Mark the incidence as changed
    if (mItem.isValid()) {
        newIncidence->setRevision(newIncidence->revision() + 1);
    }

    result.setPayload<KCalCore::Incidence::Ptr>(newIncidence);
    return result;
}

Akonadi::Collection IncidenceDialogPrivate::selectedCollection() const
{
    return mCalSelector->currentCollection();
}

void IncidenceDialogPrivate::reject(RejectReason reason, const QString &errorMessage)
{
    Q_UNUSED(reason);

    Q_Q(IncidenceDialog);
    qCCritical(INCIDENCEEDITOR_LOG) << "Rejecting:" << errorMessage;
    q->deleteLater();
}

/// IncidenceDialog

IncidenceDialog::IncidenceDialog(Akonadi::IncidenceChanger *changer,
                                 QWidget *parent, Qt::WFlags flags)
    : KDialog(parent, flags),
      d_ptr(new IncidenceDialogPrivate(changer, this))
{
    Q_D(IncidenceDialog);
    setAttribute(Qt::WA_DeleteOnClose);

    d->mUi->mTabWidget->setCurrentIndex(0);
    d->mUi->mSummaryEdit->setFocus();

    setButtons(KDialog::Ok | KDialog::Apply | KDialog::Cancel | KDialog::Default);
    setButtonToolTip(KDialog::Apply,
                     i18nc("@info:tooltip", "Save current changes"));
    setButtonToolTip(KDialog::Ok,
                     i18nc("@action:button", "Save changes and close dialog"));
    setButtonToolTip(KDialog::Cancel,
                     i18nc("@action:button", "Discard changes and close dialog"));
    setDefaultButton(KDialog::Ok);
    enableButton(Apply, false);

    setButtonText(Default, i18nc("@action:button", "&Templates..."));
    setButtonIcon(Default, QIcon::fromTheme(QStringLiteral("project-development-new-template")));
    setButtonToolTip(Default,
                     i18nc("@info:tooltip",
                           "Manage templates for this item"));
    setButtonWhatsThis(Default,
                       i18nc("@info:whatsthis",
                             "Push this button to show a dialog that helps "
                             "you manage a set of templates. Templates "
                             "can make creating new items easier and faster "
                             "by putting your favorite default values into "
                             "the editor automatically."));

    setModal(false);
    showButtonSeparator(false);

    connect(d->mUi->mAcceptInvitationButton, &QAbstractButton::clicked,
            d->mIeAttendee, &IncidenceAttendee::acceptForMe);
    connect(d->mUi->mAcceptInvitationButton, &QAbstractButton::clicked,
            d->mUi->mInvitationBar, &QWidget::hide);
    connect(d->mUi->mDeclineInvitationButton, &QAbstractButton::clicked,
            d->mIeAttendee, &IncidenceAttendee::declineForMe);
    connect(d->mUi->mDeclineInvitationButton, &QAbstractButton::clicked,
            d->mUi->mInvitationBar, &QWidget::hide);
    connect(this, SIGNAL(invalidCollection()),
            this, SLOT(slotInvalidCollection()));
    readConfig();
}

IncidenceDialog::~IncidenceDialog()
{
    writeConfig();
    delete d_ptr;
}

void IncidenceDialog::writeConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "IncidenceDialog");
    group.writeEntry("Size", size());

    const Akonadi::Collection col = d_ptr->mCalSelector->currentCollection();
    // col might not be valid if the collection wasn't found yet (the combo is async), skip saving in that case
    if (col.isValid() && col.id() != IncidenceEditorNG::IncidenceEditorSettings::self()->lastSelectedFolder()) {
        IncidenceEditorNG::IncidenceEditorSettings::self()->setLastSelectedFolder(col.id());
        IncidenceEditorNG::IncidenceEditorSettings::self()->save();
    }
}

void IncidenceDialog::readConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "IncidenceDialog");
    const QSize size = group.readEntry("Size", QSize());
    if (size.isValid()) {
        resize(size);
    } else {
        resize(QSize(500, 500).expandedTo(minimumSizeHint()));
    }
}

void IncidenceDialog::load(const Akonadi::Item &item, const QDate &activeDate)
{
    Q_D(IncidenceDialog);
    d->mIeDateTime->setActiveDate(activeDate);
    if (item.isValid()) {   // We're editing
        d->mItemManager->load(item);
        // TODO: Remove this once we support moving of events/todo's
        d->mCalSelector->setEnabled(false);
    } else { // We're creating
        Q_ASSERT(d->hasSupportedPayload(item));
        d->load(item);
        show();
    }
}

void IncidenceDialog::selectCollection(const Akonadi::Collection &collection)
{
    Q_D(IncidenceDialog);
    if (collection.isValid()) {
        d->mCalSelector->setDefaultCollection(collection);
    } else {
        d->mCalSelector->setCurrentIndex(0);
    }
}

void IncidenceDialog::setIsCounterProposal(bool isCounterProposal)
{
    Q_D(IncidenceDialog);
    d->mItemManager->setIsCounterProposal(isCounterProposal);
}

QObject *IncidenceDialog::typeAheadReceiver() const
{
    Q_D(const IncidenceDialog);
    return d->mUi->mSummaryEdit;
}

void IncidenceDialog::slotButtonClicked(int button)
{
    Q_D(IncidenceDialog);

    switch (button) {
    case KDialog::Ok: {
        if (d->isDirty() || d->mInitiallyDirty) {
            enableButtonOk(false);
            enableButtonCancel(false);
            enableButtonApply(false);
            d->mCloseOnSave = true;
            d->mInitiallyDirty = false;
            d->mItemManager->save();
        } else {
            close();
        }
        break;
    }
    case KDialog::Apply: {
        enableButtonOk(false);
        enableButtonCancel(false);
        enableButtonApply(false);

        d->mCloseOnSave = false;
        d->mInitiallyDirty = false;
        d->mItemManager->save();
        break;
    }
    case KDialog::Cancel:
        if (d->isDirty() &&
                KMessageBox::questionYesNo(
                    this,
                    i18nc("@info", "Do you really want to cancel?"),
                    i18nc("@title:window", "KOrganizer Confirmation")) == KMessageBox::Yes) {
            KDialog::reject(); // Discard current changes
        } else if (!d->isDirty()) {
            KDialog::reject(); // No pending changes, just close the dialog.
        } // else { // the user wasn't finished editting after all }
        break;
    case KDialog::Default:
        d->manageTemplates();
        break;
    default:
        Q_ASSERT(false);   // Shouldn't happen
        break;
    }
}

void IncidenceDialog::closeEvent(QCloseEvent *event)
{
    Q_D(IncidenceDialog);
    if (d->isDirty() &&
            KMessageBox::questionYesNo(
                this,
                i18nc("@info", "Do you really want to cancel?"),
                i18nc("@title:window", "KOrganizer Confirmation")) == KMessageBox::Yes) {
        KDialog::reject(); // Discard current changes
        KDialog::closeEvent(event);
    } else if (!d->isDirty()) {
        KDialog::reject(); // No pending changes, just close the dialog.
        KDialog::closeEvent(event);
    } else {
        event->ignore();
    }
}

void IncidenceDialog::setInitiallyDirty(bool initiallyDirty)
{
    Q_D(IncidenceDialog);
    d->mInitiallyDirty = initiallyDirty;
}

Akonadi::Item IncidenceDialog::item() const
{
    Q_D(const IncidenceDialog);
    return d->mItemManager->item();
}

void IncidenceDialog::handleSelectedCollectionChange(const Akonadi::Collection &collection)
{
    Q_D(IncidenceDialog);
    if (d->mItem.parentCollection().isValid()) {
        enableButton(Apply, collection.id() != d->mItem.parentCollection().id());
    }
}

#include "moc_incidencedialog.cpp"
