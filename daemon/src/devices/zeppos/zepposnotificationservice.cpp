#include "zepposnotificationservice.h"
#include "typeconversion.h"


ZeppOsNotificationService::ZeppOsNotificationService(ZeppOSDevice *device) : AbstractZeppOsService(device, true)
{
    m_endpoint = 0x001e;
}

void ZeppOsNotificationService::sendAlert(const AbstractDevice::WatchNotification &notification) const
{
    qDebug() << Q_FUNC_INFO;

#if 0
    final String senderOrTitle = StringUtils.getFirstOf(notificationSpec.sender, notificationSpec.title);

            // TODO Check real limit for notificationMaxLength / respect across all fields

            try {
                final TransactionBuilder builder = new TransactionBuilder("send notification");

                baos.write(NOTIFICATION_CMD_SEND);
                baos.write(BLETypeConversions.fromUint32(notificationSpec.getId()));
                if (notificationSpec.type == NotificationType.GENERIC_SMS) {
                    baos.write(NOTIFICATION_TYPE_SMS);
                } else {
                    baos.write(NOTIFICATION_TYPE_NORMAL);
                }
                baos.write(NOTIFICATION_SUBCMD_SHOW);

                // app package
                if (notificationSpec.sourceAppId != null) {
                    baos.write(notificationSpec.sourceAppId.getBytes(StandardCharsets.UTF_8));
                } else {
                    // Send the GB package name, otherwise the last notification icon will
                    // be used wrongly (eg. when receiving an SMS)
                    baos.write(BuildConfig.APPLICATION_ID.getBytes(StandardCharsets.UTF_8));
                }
                baos.write(0);

                // sender/title
                if (!senderOrTitle.isEmpty()) {
                    baos.write(senderOrTitle.getBytes(StandardCharsets.UTF_8));
                }
                baos.write(0);

                // body
                if (notificationSpec.body != null) {
                    baos.write(StringUtils.truncate(notificationSpec.body, maxLength()).getBytes(StandardCharsets.UTF_8));
                }
                baos.write(0);

                // app name
                if (notificationSpec.sourceName != null) {
                    baos.write(notificationSpec.sourceName.getBytes(StandardCharsets.UTF_8));
                }
                baos.write(0);

                // reply
                boolean hasReply = false;
                if (notificationSpec.attachedActions != null && !notificationSpec.attachedActions.isEmpty()) {
                    for (int i = 0; i < notificationSpec.attachedActions.size(); i++) {
                        final NotificationSpec.Action action = notificationSpec.attachedActions.get(i);

                        if (action.isReply()) {
                            hasReply = true;
                            mNotificationReplyAction.add(notificationSpec.getId(), action.handle);
                            break;
                        }
                    }
                }

                baos.write((byte) (hasReply ? 1 : 0));
                if (version >= 5) {
                    baos.write(0); // 1 for silent
                }
                if (supportsPictures) {
                    baos.write((byte) (notificationSpec.picturePath != null ? 1 : 0));
                    if (notificationSpec.picturePath != null) {
                        mNotificationPictures.add(notificationSpec.getId(), notificationSpec.picturePath);
                    }
                }
                if (supportsNotificationKey) {
                    if (notificationSpec.key != null) {
                        baos.write(notificationSpec.key.getBytes(StandardCharsets.UTF_8));
                    }
                    baos.write(0);
                }

                write(builder, baos.toByteArray());
                builder.queue(getSupport().getQueue());
            } catch (final Exception e) {
                LOG.error("Failed to send notification", e);
            }
#endif

    QByteArray cmd;

    cmd += NOTIFICATION_CMD_SEND;
    cmd += TypeConversion::fromInt32(notification.id);
    cmd += NOTIFICATION_TYPE_NORMAL;
    cmd += NOTIFICATION_SUBCMD_SHOW;

    //Application ID
    cmd += notification.appId.isEmpty() ? "uk.co.piggz.amazfish" : notification.appId;
    cmd += (uint8_t)0x00;

    //TODO Sender
    cmd += (uint8_t)0x00;

    //Body
    if (!notification.summary.isEmpty()) {
        cmd += notification.summary.toUtf8();
        cmd += 0x0a;
    }

    if (!notification.body.isEmpty()) {
        cmd += notification.body.toUtf8();
    }
    cmd += (uint8_t)0x00;

    //Sending application
    cmd += notification.appName.isEmpty() ? "Amazfish" : notification.appName;
    cmd += (uint8_t)0x00;

    //Has reply?
    cmd += (uint8_t)0x00;

    //Pad
    cmd += (uint8_t)0x00;

    //Write
    m_device->writeToChunked2021(endpoint(), cmd, encrypted());

}

void ZeppOsNotificationService::incomingCall(const QString &caller)
{
    QByteArray cmd;

    cmd += NOTIFICATION_CMD_SEND;
    cmd += TypeConversion::fromInt32(0);
    cmd += NOTIFICATION_TYPE_CALL;
    cmd += NOTIFICATION_CALL_STATE_START;
    cmd += (uint8_t)0x00;
    cmd += caller.toUtf8();
    cmd += (uint8_t)0x00;
    cmd += (uint8_t)0x00;
    cmd += (uint8_t)0x00;

    cmd += (uint8_t)0x00;
    cmd += (uint8_t)0x00;
    m_device->writeToChunked2021(endpoint(), cmd, encrypted());
}

void ZeppOsNotificationService::handlePayload(const QByteArray &payload)
{

}

QString ZeppOsNotificationService::name() const
{
    return "NotificationService";
}
