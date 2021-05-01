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
    [[nodiscard]] QString path() const;
    void setPath(const QString &path);

signals:
    void finished();
    void newStepStarted(const QString &step);
    void stepMaxValChanged(int maxVal);
    void progressValChanged(int curVal);
    void noDupesFound();
    void foundDuplicate(QString crc, QStringList files);
};

#endif // DUPEFINDERCRC_H
