#pragma once

#include <QtCore/QByteArray>
#include <QtCore/QVector>
#include <QtCore/QString>
#include <QtCore/QtGlobal>

#include "garmintypes.h"
#include "garmingfdimessage.h"
#include "communicator_v2.h"

#include "libwatchfish/calendarsource.h"


struct CalendarServiceRequest
{
    bool useCoreServiceEnvelope = false;

    quint64 startDate = 0;
    quint64 endDate = 0;

    bool includeOrganizer = false;
    bool includeTitle = true;
    bool includeLocation = true;
    bool includeDescription = false;
    bool includeStartDate = true;
    bool includeEndDate = false;
    bool includeAllDay = false;

    quint32 maxOrganizerLength = 0;
    quint32 maxTitleLength = 0;
    quint32 maxLocationLength = 0;
    quint32 maxDescriptionLength = 0;
    quint32 maxEvents = 100;
};

// -----------------------------------------------------------------------------
// Calendar event in protobuf format
// -----------------------------------------------------------------------------
struct CalendarEventProto
{
    bool hasOrganizer = false;
    QString organizer;

    QString title;

    bool hasLocation = false;
    QString location;

    bool hasDescription = false;
    QString description;

    QDateTime startDate ;
    QDateTime endDate;
    bool allDay = false;

    QVector<quint32> reminderTimes;
};

// -----------------------------------------------------------------------------
// Response status
// -----------------------------------------------------------------------------
enum class CalendarResponseStatus : quint32
{
    UnknownStatus = 0,
    Ok = 1,
    InvalidDateRange = 2,
};



class  GarminCalendarMessage : public GarminGfdiMessage
{
    Q_OBJECT
 public:
    GarminCalendarMessage(CommunicatorV2 *parent) {
        mCommunicator = parent;
    }
    void parse(const QByteArray& data, quint16 requestId, quint32 dataOffset);

    Result<QVector<CalendarEventProto>> handleCalendarRequest(
        const CalendarServiceRequest& request);

    QByteArray encodeCalendarResponse(
        const QVector<CalendarEventProto>& events,
        CalendarResponseStatus status,
        quint16 requestId,
        bool useCoreServiceEnvelope);

 private:

    bool parseField(const QByteArray& data,
                    int startOffset,
                    quint32& fieldNum,
                    quint8& wireType,
                    QByteArray& fieldData,
                    int& nextCursor);

    Result<QPair<quint64, int>> decodeVarint(const QByteArray& data);
    void encodeVarint(QByteArray& buffer, quint64 value);
    void encodeFieldKey(QByteArray& buffer, quint32 fieldNum, quint8 wireType);

    QByteArray encodeCalendarEvent(const CalendarEventProto& event);


};




