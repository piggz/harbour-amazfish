#include "activityfetchoperation.h"

#include <QDir>
#include <QDebug>

ActivityFetchOperation::ActivityFetchOperation(const QDateTime &sd)
{
    if (!QDir(QDir::homePath() + "/amazfish").exists()) {
        qDebug() << "Creating amazfish folder";
        if (!QDir(QDir::homePath()).mkdir("amazfish")) {
            qDebug() << "Error creating amazfish folder!";
            return;
        }
    }

    m_startDate = sd;

    QString filename = "amazfitbip_activity.log";
    m_logFile = new QFile(QDir::homePath() + "/amazfish/" + filename);

    if(m_logFile->open(QIODevice::Append)) {
        m_dataStream = new QTextStream(m_logFile);
    }
}

void ActivityFetchOperation::newData(const QByteArray &data)
{
    int len = data.length();

    if (len % 4 != 1) {
        qDebug() << "Unexpected data size";
        return;
    }

    for (int i = 1; i < len; i+=4) {
        ActivitySample sample(data[i] & 0xff, data[i + 1] & 0xff, data[i + 2] & 0xff, data[i + 3] & 0xff);
        m_samples << (sample);
    }
}

void ActivityFetchOperation::finished(bool success)
{
    if (success) {
        //store the successful samples
        saveSamples();
        m_settings.setValue("/uk/co/piggz/amazfish/device/lastActivitySyncMillis", m_sampleTime.toMSecsSinceEpoch());
    }
    if (m_logFile) {
        m_logFile->close();
    }
}

void ActivityFetchOperation::saveSamples()
{
    if (m_samples.count() > 0) {
        if (m_dataStream) {
            m_sampleTime = m_startDate;

            QString dev =  m_settings.value("/uk/co/piggz/amazfish/pairedAddress").toString();
            QString profileName = m_settings.value("/uk/co/piggz/amazfish/profile/name").toString();

            uint id = qHash(profileName);

            for (int i = 0; i < m_samples.count(); ++i) {
                *m_dataStream << dev << "," << id << "," << m_sampleTime.toMSecsSinceEpoch() / 1000 << ",";
                m_samples[i].write(*m_dataStream);
                *m_dataStream << "\n";

                m_sampleTime.addSecs(60);
            }
        }
    }
}

void ActivityFetchOperation::setStartDate(const QDateTime &sd)
{
    m_startDate = sd;
}
