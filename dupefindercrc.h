#ifndef DUPEFINDERCRC_H
#define DUPEFINDERCRC_H

#include <QObject>
#include <vector>
#include "karaokefile.h"

class DupeFinderCRC : public QObject
{
    Q_OBJECT
    QString m_path;
public:
    explicit DupeFinderCRC(QObject *parent = nullptr);
    void findDupes();
    QString path() const;
    void setPath(const QString &path);

signals:
    void finished();
    void findingFilesStarted();
    void foundFiles(int);
    void gettingChecksumsStarted(int);
    void gettingChecksumsProgress(int);
    void dupeFindStarted(int);
    void dupeFindProgress(int);
    void foundDuplicate(QString crc, QStringList files);
};

#endif // DUPEFINDERCRC_H
