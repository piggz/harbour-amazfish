#include "uihhcontainer.h"

#include "typeconversion.h"
#include "amazfish.h"
#include <qdebug.h>


const QMap<QByteArray, char> fileTypeMap = {
    {"lto7dv5.brm", 0x8b},
    {"EPO_GR_3.DAT", -116},
    {"EPO_GAL_7.DAT", -115},
    {"EPO_BDS_3.DAT", -114},
};

UIHHContainer::UIHHContainer()
{

}

void UIHHContainer::addFile(File file)
{
    m_files << file;
}

QByteArray UIHHContainer::toRawBytes()
{
    QByteArray content;

    for (File f: m_files) {
        content += f.header();
        content += f.content();
    }

    QByteArray header = buildHeader(content);

    return header + content;
}

QByteArray UIHHContainer::buildHeader(const QByteArray &content)
{
    QByteArray header;
    header += 'U';
    header += 'I';
    header += 'H';
    header += 'H';

    header += 0x04;
    header += (char)0x00;
    header += (char)0x00;
    header += (char)0x00;
    header += (char)0x00;
    header += (char)0x00;
    header += (char)0x00;
    header += 0x01;

    header += TypeConversion::fromInt32(Amazfish::calculateCRC32(content));

    header += (char)0x00;
    header += (char)0x00;
    header += (char)0x00;
    header += (char)0x00;
    header += (char)0x00;
    header += (char)0x00;

    header += TypeConversion::fromInt32(content.length());

    header += (char)0x00;
    header += (char)0x00;
    header += (char)0x00;
    header += (char)0x00;
    header += (char)0x00;
    header += (char)0x00;

    return header;
}

UIHHContainer::File::File(QByteArray name, QByteArray data) : m_fileName(name), m_fileData(data)
{
    qDebug() << Q_FUNC_INFO << m_fileName << m_fileData.length();
    m_type = fileTypeMap[name];
}

QByteArray UIHHContainer::File::header() const
{
    qDebug() << Q_FUNC_INFO << m_fileData.length();

    QByteArray header;

    header += 0x01;
    header += m_type;
    header += TypeConversion::fromInt32(m_fileData.length());
    header += TypeConversion::fromInt32(Amazfish::calculateCRC32(m_fileData));

    return header;
}

QByteArray UIHHContainer::File::content() const
{
    return m_fileData;
}
