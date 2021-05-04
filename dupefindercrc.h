#ifndef DUPEFINDERCRC_H
#define DUPEFINDERCRC_H

#include <QObject>
#include <vector>
#include "karaokefile.h"
#include <memory>

class DupeFinderCRC : public QObject
{
    Q_OBJECT
    QString m_path;
    
public:
    explicit DupeFinderCRC(QObject *parent = nullptr);
    void findDupes();
    void setPath(const QString &path);

signals:
    void finished();
    void newStepStarted(const QString &step);
    void stepMaxValChanged(int maxVal);
    void progressValChanged(int curVal);
    void noDupesFound();
    void foundDuplicates(QVector<QSharedPointer<KaraokeFile>> kFiles);
    void foundBadFiles(QVector<QSharedPointer<KaraokeFile>> kFiles);
};

#endif // DUPEFINDERCRC_H
