#include "dupefindercrc.h"
#include <set>
#include <algorithm>
#include <QDirIterator>
#include <mutex>
#include "karaokefile.h"
#include <spdlog/spdlog.h>
#include <QSharedPointer>

QString DupeFinderCRC::path() const {
    return m_path;
}

void DupeFinderCRC::setPath(const QString &path) {
    m_path = path;
}

DupeFinderCRC::DupeFinderCRC(QObject *parent) : QObject(parent) {
}

void DupeFinderCRC::findDupes() {
    std::set<uint32_t> crcChecksums;
    emit newStepStarted("Finding karaoke files...");
    spdlog::info("Getting files in path: {}", m_path.toStdString());
    QStringList files;
    QDirIterator it(m_path, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        if (it.fileInfo().completeSuffix().toLower() == "zip") {
            files.push_back(it.filePath());
        }
    }
    spdlog::info("Got all files, calculating crc32 checksums");
    emit newStepStarted("Calculating crc32 checksums");
    emit stepMaxValChanged(files.size());
    QVector<QSharedPointer<KaraokeFile>> kFiles;
    kFiles.reserve(files.size());
    int processedFiles{0};
    for (const auto &file : files) {
        auto kFile = QSharedPointer<KaraokeFile>(new KaraokeFile(file));
        emit progressValChanged(++processedFiles);
        crcChecksums.insert(kFile->crc());
        kFiles.push_back(kFile);
    }
    spdlog::info("Checksums calculated, finding duplicates");
    emit newStepStarted("Scanning for duplicate checksums...");
    emit stepMaxValChanged((int) crcChecksums.size());
    int processedChecksums{0};
    uint dupesFound{0};
    spdlog::info("Checking for bad files...");
    QVector<QSharedPointer<KaraokeFile>> badFiles;
    std::copy_if(kFiles.begin(), kFiles.end(), std::back_inserter(badFiles), [](auto file) {
        return (file->crc() == 0);
    });
    kFiles.erase(std::remove_if(kFiles.begin(), kFiles.end(), [] (auto file) {
        return (file->crc() == 0);
    }), kFiles.end());
    if (!badFiles.empty()) {
        spdlog::warn("Found {} bad zip files", badFiles.size());
        emit foundBadFiles(badFiles);
    }
    else
        spdlog::info("No bad files detected");
    for (const auto crc : crcChecksums) {
        QVector<QSharedPointer<KaraokeFile>> newVec;
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
