#ifndef UIHHCONTAINER_H
#define UIHHCONTAINER_H

#include <QMap>
#include <QByteArray>

class UIHHContainer
{
public:
    class File {
    public:
        File(QByteArray name, QByteArray data);

        QByteArray header() const;
        QByteArray content() const;

    private:
        QByteArray m_fileName;
        QByteArray m_fileData;
        char m_type;
    };

    UIHHContainer();
    void addFile(File file);
    QByteArray toRawBytes();

private:
    QList<File> m_files;

    QByteArray buildHeader(const QByteArray &contet);
};

#endif // UIHHCONTAINER_H
