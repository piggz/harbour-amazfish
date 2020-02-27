#ifndef CALENDARREADER_H
#define CALENDARREADER_H

#include <QObject>
#include <QString>
#include <extendedcalendar.h>
#include <extendedstorage.h>

class CalendarReader : public QObject, public mKCal::ExtendedStorageObserver
{
    Q_OBJECT
public:
    struct EventData {
        QString calendarUid;
        QString uniqueId;
        QString recurrenceId;
        QDateTime startTime;
        QDateTime endTime;
        bool allDay;
        QString color;
        QString displayLabel;
        QString description;
        QString location;

    };

    CalendarReader();
    ~CalendarReader();

    QList<EventData> getEvents();

protected:
    void storageModified(mKCal::ExtendedStorage *storage, const QString &info) Q_DECL_OVERRIDE;
    void storageProgress(mKCal::ExtendedStorage *storage, const QString &info) Q_DECL_OVERRIDE;
    void storageFinished(mKCal::ExtendedStorage *storage, bool error, const QString &info) Q_DECL_OVERRIDE;

private:
    QString normalizeCalendarName(QString name);
    mKCal::ExtendedCalendar::Ptr _calendar;
    mKCal::ExtendedStorage::Ptr _calendarStorage;
};

#endif // CALENDARREADER_H
