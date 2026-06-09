#include "garminnotificationcontrolmessage.h"

void GarminNotificationControlMessage:: parse(const QByteArray& data) {
    qDebug() << Q_FUNC_INFO << "Garmin: parsing notification control";
    if (data.size() < 7) {
        return;
    }

    NotificationControlMessage msg;
    msg.command = quint8(data[0]);
    msg.notificationId = i32le(data, 1);

    // PERFORM_NOTIFICATION_ACTION (128)
    if (msg.command == 128) {
        if (data.size() < 6) {
            return;
        }
        msg.actionId = quint8(data[5]);

        // null-terminated action string (if present)
        if (data.size() > 6) {
            const QByteArray stringData = data.mid(6);
            const int nullPos = stringData.indexOf(char(0));
            if (nullPos > 0) {
                const QByteArray raw = stringData.left(nullPos);
                const QString s = QString::fromUtf8(raw);
                if (!s.isEmpty() && s.toUtf8() == raw) {
                    bool printable = true;
                    for (QChar c : s) {
                        if (c.isPrint()) continue;
                        if (c == QChar('\n') || c == QChar('\r')) continue;
                        printable = false;
                        break;
                    }
                    if (printable) msg.actionString = s;
                }
            }
        }
        // attributes empty
        if (mCommunicator)mCommunicator->onNotificationControlReceived(msg);
        return;
    }

    // Requested attributes parsing
    int off = 5;
    while (off < data.size()) {
        if (off + 3 > data.size()) break;
        const quint8 attrId = quint8(data[off]);

        if (attrId == 0 || attrId == 4 || attrId == 5) {
            msg.attributes.append({attrId, 128});
            off += 1;
            continue;
        }

        if (attrId == 127) {
            msg.attributes.append({attrId, quint16(quint8(data[off + 1]))});
            off += 1; // attr_id
            off += 1; // short
            off += 2; // byte;
            continue;
        }

        const quint16 maxLen = u16le(data, off + 1);
        msg.attributes.append({attrId, maxLen});
        off += 3;
    }
    if (mCommunicator) mCommunicator->onNotificationControlReceived(msg);
}
