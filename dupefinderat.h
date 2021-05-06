#ifndef DUPEFINDERAT_H
#define DUPEFINDERAT_H

#include <QObject>
#include <vector>
#include "karaokefile.h"


class DupeFinderAT : public QObject
{
    Q_OBJECT
    KLM::KaraokePathList m_paths;
    
public:
    explicit DupeFinderAT(QObject *parent = nullptr);
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

#endif // DUPEFINDERAT_H
