#include "zepposfirmwareinfo.h"
#include "uihhcontainer.h"
#include <KArchive>
#include <kzip.h>
#include <KCompressionDevice>

ZeppOSFirmwareInfo::ZeppOSFirmwareInfo(const QByteArray &bytes, const QString &path) : HuamiFirmwareInfo()
{
    m_bytes = bytes;
    m_fileName = path;

    calculateCRC16();
    calculateCRC32();
    determineFirmwareType();
    determineFirmwareVersion();

    qDebug() << Q_FUNC_INFO << m_type << m_version << m_crc16 << m_crc32;
}

bool ZeppOSFirmwareInfo::supportedOnDevice(const QString &device) const
{
    return true;
}

bool ZeppOSFirmwareInfo::isEpoZip()
{
    qDebug() << Q_FUNC_INFO;
    QDataStream in(&m_bytes, QIODevice::ReadOnly);
    KCompressionDevice dev(in.device(), false, KCompressionDevice::CompressionType::None);
    KZip zip(&dev);

    if(!zip.open(QIODevice::ReadOnly))
    {
        qDebug() << "Cannot open the firmware archive";
        return false;
    }

    const auto* root = zip.directory();
    qDebug() << root->entries();

    const auto* metaEntry = dynamic_cast<const KArchiveDirectory*>(root->entry("META-INF"));
    if(metaEntry == nullptr)
    {
        qDebug() << "Archive does nto contain a META-INFO folder";
        return false;
    }
    if (metaEntry->entries().contains("MANIFEST.MF") && root->entries().contains("EPO_BDS_3.DAT")
            && root->entries().contains("EPO_GAL_7.DAT") && root->entries().contains("EPO_GR_3.DAT")) {
        qDebug() << "Archive looks like an EPO zip filer";
        return true;
    }

    return false;
}

bool ZeppOSFirmwareInfo::buildEpoUIHH()
{
    qDebug() << Q_FUNC_INFO;
    QDataStream in(&m_bytes, QIODevice::ReadOnly);
    KCompressionDevice dev(in.device(), false, KCompressionDevice::CompressionType::None);
    KZip zip(&dev);

    if(!zip.open(QIODevice::ReadOnly))
    {
        qDebug() << "Cannot open the firmware archive";
        return false;
    }

    const auto* root = zip.directory();

    UIHHContainer uih;

    auto zipData = [root](const QString &entry) {
        auto f = dynamic_cast<const KZipFileEntry*>(root->entry(entry));

        if (!f) {
            qDebug() << "File not found:" << entry << root->entries();
            return QByteArray();
        }
        return f->data();
    };

    UIHHContainer::File bds("EPO_BDS_3.DAT", zipData("EPO_BDS_3.DAT"));
    uih.addFile(bds);

    UIHHContainer::File gal("EPO_GAL_7.DAT", zipData("EPO_GAL_7.DAT"));
    uih.addFile(gal);

    UIHHContainer::File gr("EPO_GR_3.DAT", zipData("EPO_GR_3.DAT"));
    uih.addFile(gr);

    m_bytes = uih.toRawBytes();
    return true ;
}

void ZeppOSFirmwareInfo::determineFirmwareType()
{
    qDebug() << Q_FUNC_INFO << "Determining firmware type";
    m_type = Invalid;

    if (m_bytes.startsWith(UCHARARR_TO_BYTEARRAY(AGPS_UIHH_HEADER))) {
        m_type = GPS_UIHH;
    } else if (m_bytes.startsWith(UCHARARR_TO_BYTEARRAY(PKZIP_HEADER))) {
        if (isEpoZip() && buildEpoUIHH()) {
            m_type = GPS_UIHH;
        }
    } else if (path().fileName() == "lto7dv5.brm") {
        m_type = GPS_UIHH;
        UIHHContainer uih;
        UIHHContainer::File uif(path().fileName().toLocal8Bit(), m_bytes);
        uih.addFile(uif);

        m_bytes = uih.toRawBytes();
    }
}

void ZeppOSFirmwareInfo::determineFirmwareVersion()
{
    QString version;

    switch (m_type) {
    case GPS_UIHH:
        version = "GPS_UIHH";
        break;
    case Watchface:
        version = "Watchface";
        break;
    default:
        version = "(unknown)";
    }

    m_version = version;
}
