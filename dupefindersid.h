#ifndef DUPEFINDERSID_H
#define DUPEFINDERSID_H

#include <QObject>
#include <vector>
#include "karaokefile.h"
#include <memory>

class DupeFinderSID : public QObject
{
    Q_OBJECT
    QString m_path;
    
public:
    explicit DupeFinderSID(QObject *parent = nullptr);
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

#endif // DUPEFINDERSID_H
