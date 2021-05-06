#include "dupefindersid.h"
#include <set>
#include <algorithm>
#include <QDirIterator>
#include <mutex>
#include "karaokefile.h"
#include <spdlog/spdlog.h>


void DupeFinderSID::setPaths(KLM::KaraokePathList paths)
{
    m_paths = paths;
}

DupeFinderSID::DupeFinderSID(QObject *parent) : QObject(parent) {
}

void DupeFinderSID::findDupes() {
    std::set<QString> songIDs;
    emit newStepStarted("Finding karaoke files...");
    emit stepMaxValChanged(0);
    KLM::KaraokeFileList kFiles;
    for ( auto path : m_paths )
    {
        spdlog::info("Getting files in path: {}", path->path().toStdString());
        kFiles.append(path->files());
        spdlog::info("Found {} karaoke files", path->files().size());
    }
    spdlog::info("Found a totoal of {} karaoke files in all paths, getting songIDs", kFiles.size());
    emit newStepStarted("Getting song ID data...");
    emit stepMaxValChanged(kFiles.size());
    int processedFiles{0};
    for (const auto &kFile : kFiles) {
        emit progressValChanged(++processedFiles);
        kFile->setNamingPattern(NamingPattern::PatternSAT());
        songIDs.insert(kFile->songid());
    }
    spdlog::info("Got artist/title data, finding duplicates");
    emit newStepStarted("Scanning for duplicates...");
    emit stepMaxValChanged((int) songIDs.size());
    int processedCombos{0};
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
    for (const auto &songID : songIDs) {
        KLM::KaraokeFileList newVec;
        if (songID == "") {
            emit progressValChanged(++processedCombos);
            continue;
        }
        std::copy_if(kFiles.begin(), kFiles.end(), std::back_inserter(newVec), [songID](auto file) {
            return (file->songid() == songID);
        });
        kFiles.erase(std::remove_if(kFiles.begin(), kFiles.end(), [songID](auto file) {
            return (file->songid() == songID);
        }), kFiles.end());
        if (newVec.size() > 1) {
            for (const auto& kFile : newVec)
                kFile->getBitrate();
            emit foundDuplicates(newVec);
            dupesFound++;
        }
        emit progressValChanged(++processedCombos);
    }
    if (dupesFound == 0)
            emit noDupesFound();
    spdlog::info("Processing complete");
    kFiles.clear();
    kFiles.shrink_to_fit();
    emit finished();
}
