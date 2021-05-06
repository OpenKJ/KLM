#ifndef DUPEFINDERCRC_H
#define DUPEFINDERCRC_H

#include <QObject>
#include <vector>
#include "karaokefile.h"


class DupeFinderCRC : public QObject
{
    Q_OBJECT
    KLM::KaraokePathList m_paths;
    
public:
    explicit DupeFinderCRC(QObject *parent = nullptr);
    void findDupes();
    void setPaths(KLM::KaraokePathList paths);

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
