#include "dupefindercrc.h"
#include <set>
#include <algorithm>
#include <execution>
#include <QDebug>
#include <QDirIterator>
#include <mutex>
#include "karaokefile.h"

QString DupeFinderCRC::path() const
{
    return m_path;
}

void DupeFinderCRC::setPath(const QString &path)
{
    m_path = path;
}

DupeFinderCRC::DupeFinderCRC(QObject *parent) : QObject(parent)
{

}

void DupeFinderCRC::findDupes()
{
    std::set<uint32_t> crcChecksums;
    emit findingFilesStarted();
    qWarning() << "Getting files in " << m_path;
    QStringList files;
    QDirIterator it(m_path, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        if (it.fileInfo().completeSuffix().toLower() == "zip")
        {
            files.push_back(it.filePath());
        }
    }
    emit foundFiles(files.size());
    qWarning() << "Got all files, calculating checksums";
    emit gettingChecksumsStarted(files.size());
    std::vector<KaraokeFile*> kfiles;
    kfiles.reserve(files.size());
    int processedFiles{0};
    for (const auto &file : files) {
        KaraokeFile *kfile = new KaraokeFile;
        kfile->setPath(file);
        auto checksum = kfile->crc();
        emit gettingChecksumsProgress(++processedFiles);
        crcChecksums.insert(checksum);
        kfiles.push_back(kfile);
    }
    qWarning() << "Checksums calculated, finding possible duplicates";
    emit dupeFindStarted(crcChecksums.size());
    int processedChecksums{0};
    uint dupesFound{0};
    for (const auto crc : crcChecksums)
    {
        std::vector<KaraokeFile*> newvec;
        if (crc == 0) {
            emit dupeFindProgress(++processedChecksums);
            continue;
        }
        std::copy_if(kfiles.begin(), kfiles.end(), std::back_inserter(newvec), [crc] (auto file) {
            return (file->crc() == crc);
        });
        kfiles.erase(std::remove_if(kfiles.begin(), kfiles.end(), [crc] (auto file) {
            return (file->crc() == crc);
        }), kfiles.end());
        if (newvec.size() > 1)
        {
            QStringList paths;
            for (const auto &file : newvec)
            {
                paths.push_back(file->path());
            }
            QString hexvalue = QString("%1").arg(crc, 0, 16, QLatin1Char( '0' ));
            emit foundDuplicate(hexvalue, paths);
            dupesFound++;
        }
        emit dupeFindProgress(++processedChecksums);
    }
    if (dupesFound == 0)
        emit noDupesFound();
    qWarning() << "Done";
    for (auto kfile : kfiles)
        delete kfile;
    kfiles.clear();
    kfiles.shrink_to_fit();
    emit finished();
}
