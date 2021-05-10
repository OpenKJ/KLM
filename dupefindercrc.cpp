#include "dupefindercrc.h"
#include <set>
#include <algorithm>
#include <QDirIterator>
#include <mutex>
#include "karaokefile.h"
#include <spdlog/spdlog.h>
#include <QtConcurrent>
#include <QApplication>
#include <functional>


void DupeFinderCRC::setPaths(KLM::KaraokePathList paths)
{
    m_paths = paths;
}

DupeFinderCRC::DupeFinderCRC(QObject *parent) : QObject(parent) {
}

void DupeFinderCRC::findDupes() {
    emit newStepStarted("Finding karaoke files...");
    emit stepMaxValChanged(0);
    KLM::KaraokeFileList kFiles;

    for ( auto path : m_paths )
    {
        spdlog::info("Getting files in path: {}", path->path().toStdString());
        kFiles.append(path->files());
        spdlog::info("Found {} karaoke files", path->files().size());
    }
    spdlog::info("Found a totoal of {} karaoke files in all paths, calculating crc32 checksums", kFiles.size());
    emit newStepStarted("Calculating crc32 checksums");
    emit stepMaxValChanged(kFiles.size());
    QFutureWatcher<QSet<uint32_t>> watch;
    std::function<uint32_t(QSharedPointer<KaraokeFile>)> func = [] (QSharedPointer<KaraokeFile> kFile) -> uint32_t { return kFile->crc(); };
    connect(&watch, &QFutureWatcher<QSet<uint32_t>>::progressValueChanged, this, &DupeFinderCRC::progressValChanged);
    QFuture<QSet<uint32_t>> fut = QtConcurrent::mappedReduced(kFiles, func, &QSet<uint32_t>::insert);
    watch.setFuture(fut);
    while (!fut.isFinished())
        QApplication::processEvents();
    QSet<uint32_t> crcChecksums = fut.result();
    spdlog::info("Checksums calculated, finding duplicates");
    emit newStepStarted("Scanning for duplicate checksums...");
    emit stepMaxValChanged((int) crcChecksums.size());
    int processedChecksums{0};
    uint dupesFound{0};
    spdlog::info("Checking for bad files...");
    KLM::KaraokeFileList badFiles;
    std::copy_if(kFiles.begin(), kFiles.end(), std::back_inserter(badFiles), [] (auto file) {
        return (file->getErrorCode() != KaraokeFile::ErrorCode::OK);
    });
    kFiles.erase(std::remove_if(kFiles.begin(), kFiles.end(), [] (auto file) {
        return (file->getErrorCode() != KaraokeFile::ErrorCode::OK);
    }), kFiles.end());
    if (!badFiles.empty()) {
        spdlog::warn("Found {} bad zip files", badFiles.size());
        emit foundBadFiles(badFiles);
    }
    else
        spdlog::info("No bad files detected");
    for (const auto crc : crcChecksums) {
        KLM::KaraokeFileList newVec;
        if (crc == 0) {

            emit progressValChanged(++processedChecksums);
            continue;
        }
        std::copy_if(kFiles.begin(), kFiles.end(), std::back_inserter(newVec), [crc](auto file) {
            return (file->crc() == crc);
        });
        kFiles.erase(std::remove_if(kFiles.begin(), kFiles.end(), [crc](auto file) {
            return (file->crc() == crc);
        }), kFiles.end());
        if (newVec.size() > 1) {
            for (const auto& kFile : newVec)
                kFile->getBitrate();
            emit foundDuplicates(newVec);
            dupesFound++;
        }
        emit progressValChanged(++processedChecksums);
    }
    if (dupesFound == 0)
            emit noDupesFound();
    spdlog::info("Processing complete");
    kFiles.clear();
    kFiles.shrink_to_fit();
    emit finished();
}
