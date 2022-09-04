#include "infinitimefirmwareinfo.h"

#include <KArchive>
#include <kzip.h>
#include <KCompressionDevice>

InfinitimeFirmwareInfo::InfinitimeFirmwareInfo(const QByteArray &bytes)
{
    m_bytes = bytes;

    calculateCRC16();
    determineFirmwareType();
    determineFirmwareVersion();

    //qDebug() << mBytes;
    qDebug() << m_type << m_version << m_crc16;
}

bool InfinitimeFirmwareInfo::supportedOnDevice(const QString &device) const
{
    return (device == "InfiniTime" || device == "Pinetime-JF") && m_type != Invalid && !m_version.contains("unknown");;
}

void InfinitimeFirmwareInfo::determineFirmwareType()
{
    qDebug() << "Determining firmware type";
    m_type = Invalid;

    if (m_bytes.startsWith(UCHARARR_TO_BYTEARRAY(ZIP_HEADER))) {
        QDataStream in(&m_bytes, QIODevice::ReadOnly);
        KCompressionDevice dev(in.device(), false, KCompressionDevice::CompressionType::None);
        KZip zip(&dev);

        if(zip.open(QIODevice::ReadOnly))
        {
            auto* root = zip.directory();
            if(root->entry("manifest.json") != nullptr)
            {
                qDebug() << "DFU file detected";
                m_type = Firmware;
            }
        }
    }
}

void InfinitimeFirmwareInfo::determineFirmwareVersion()
{
    switch (m_type) {
    case Firmware:
        m_version = "FW ()";
        break;
    default:
        m_version = "unknown";
    }
}
