#include "calendarreader.h"
#include <QDebug>
#include <QSettings>

CalendarReader::CalendarReader() : _calendar(new mKCal::ExtendedCalendar(KDateTime::Spec::LocalZone())),
    _calendarStorage(_calendar->defaultStorage(_calendar))
{
    _calendarStorage->registerObserver(this);
    if (!_calendarStorage->open()) {
        qWarning() << "Cannot open calendar database";
    }
}

CalendarReader::~CalendarReader()
{
    _calendarStorage->unregisterObserver(this);
}

QString CalendarReader::normalizeCalendarName(QString name)
{
    if (name == "qtn_caln_personal_caln") {
        return QObject::tr("Personal");
    }

    return name;
}

QList<CalendarReader::EventData> CalendarReader::getEvents()
{
    QDate startDate = QDate::currentDate();
    QDate endDate = QDate::currentDate().addDays(14);
    QList<EventData> eventlist;

    _calendarStorage->loadRecurringIncidences();
    _calendarStorage->load(startDate, endDate);
    qDebug() << "Refreshing organizer from" << startDate.toString() << "to" << endDate.toString();


    //We have to use it to detect which calendars have been turned off, and
    QSettings nemoSettings("nemo", "nemo-qml-plugin-calendar");

    auto events = _calendar->rawExpandedEvents(startDate, endDate, true, true);
    qDebug() << events.count();
    for (const auto &expanded : events) {
        const QDateTime &start = expanded.first.dtStart;
        KCalCore::Incidence::Ptr incidence = expanded.second;
        EventData event;

        mKCal::Notebook::Ptr notebook = _calendarStorage->notebook(_calendar->notebook(incidence));
        if (notebook) {
            if (nemoSettings.value("exclude/"+notebook->uid()).toBool()) {
                qDebug() << "Event " << incidence->summary() << " ignored because calendar " << notebook->name() << " excluded. ";
                continue;
            }
        }

        event.uniqueId = incidence->uid();
        event.allDay = incidence->allDay();
        event.description = incidence->description();
        event.displayLabel = incidence->summary();
        event.startTime = start;
        event.location = incidence->location();

        eventlist << event;
    }

    return eventlist;
}

void CalendarReader::storageModified(mKCal::ExtendedStorage *storage, const QString &info)
{
    Q_UNUSED(storage);
    Q_UNUSED(info);
}

void CalendarReader::storageProgress(mKCal::ExtendedStorage *storage, const QString &info)
{
    Q_UNUSED(storage);
    Q_UNUSED(info);
    // Nothing to do
}

void CalendarReader::storageFinished(mKCal::ExtendedStorage *storage, bool error, const QString &info)
{
    Q_UNUSED(storage);
    Q_UNUSED(error);
    Q_UNUSED(info);
    // Nothing to do
}
