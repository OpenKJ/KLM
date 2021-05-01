#include "karaokefileinfo.h"
#include <QRegularExpression>
#include <QDebug>
#include <QTemporaryDir>
#include "tagreader.h"
#include "mzarchive.h"
#include <QSqlQuery>

void KaraokeFileInfo::readTags() {
    if (m_tagsRead)
        return;
    auto *tagReader = new TagReader(this);

    if (m_fileName.endsWith(".cdg", Qt::CaseInsensitive)) {
        QString baseFn = m_fileName;
        QString mediaFile;
        baseFn.chop(3);
        if (QFile::exists(baseFn + "mp3"))
            mediaFile = baseFn + "mp3";
        else if (QFile::exists(baseFn + "Mp3"))
            mediaFile = baseFn + "Mp3";
        else if (QFile::exists(baseFn + "MP3"))
            mediaFile = baseFn + "MP3";
        else if (QFile::exists(baseFn + "mP3"))
            mediaFile = baseFn + "mP3";
        tagReader->setMedia(mediaFile);
        m_tagArtist = tagReader->getArtist();
        m_tagTitle = tagReader->getTitle();
        m_tagSongid = tagReader->getAlbum();
        QString track = tagReader->getTrack();
        if (track != "") {
            m_tagSongid.append("-" + track);
        }
    } else if (m_fileName.endsWith(".zip", Qt::CaseInsensitive)) {
        MzArchive archive;
        QTemporaryDir dir;
        archive.setArchiveFile(m_fileName);
        archive.checkAudio();
        QString audioFile = "temp" + archive.audioExtension();
        archive.extractAudio(dir.path(), audioFile);
        tagReader->setMedia(dir.path() + QDir::separator() + audioFile);
        m_tagArtist = tagReader->getArtist();
        m_tagTitle = tagReader->getTitle();
        m_tagSongid = tagReader->getAlbum();
        m_duration = archive.getSongDuration();
        QString track = tagReader->getTrack();
        if (track != "") {
            m_tagSongid.append("-" + track);
        }
    } else {
        qInfo() << "KaraokeFileInfo::readTags() called on non zip or cdg file (" << m_fileName << ").  Trying taglib.";
        tagReader->setMedia(m_fileName);
        m_tagArtist = tagReader->getArtist();
        m_tagTitle = tagReader->getTitle();
        m_tagSongid = tagReader->getAlbum();
        m_duration = tagReader->getDuration();
//        m_tagArtist = "Error";
//        m_tagTitle = "Error";
//        m_tagSongid = "Error";
//        duration = 0;
    }
    m_tagsRead = true;
    delete tagReader;
}

void KaraokeFileInfo::setFileName(const QString &filename) {
    m_fileName = filename;
    m_fileBaseName = QFileInfo(filename).completeBaseName();
    m_tagArtist = "";
    m_tagTitle = "";
    m_tagSongid = "";
    m_artist = "";
    m_title = "";
    m_songId = "";
    m_crc32 = 0;
    m_duration = 0;
    m_tagsRead = false;
}

void KaraokeFileInfo::setPattern(int pattern, const QString &path) {
//    m_useMetadata = false;
//    int customPatternId = 0;
    m_artist = "";
    m_title = "";
    m_songId = "";
    this->m_path = path;
    this->m_pattern = pattern;
//    switch (pattern)
//    {
//    case SourceDir::STA:
////        setSongIdRegEx("^.+?(?=(\\s|_)-(\\s|_))");
////        setTitleRegEx("(?<=(\\s|_)-(\\s|_))(.*?)(?=(\\s|_)-(\\s|_))", 0);
////        setArtistRegEx("(?:^\\S+(?:\\s|_)-(?:\\s|_).+(?:\\s|_)-(?:\\s|_))(.+)",1);
//        break;
//    case SourceDir::SAT:
////        setSongIdRegEx("^.+?(?=(\\s|_)-(\\s|_))");
////        setTitleRegEx("(?:^\\S+(?:\\s|_)-(?:\\s|_).+(?:\\s|_)-(?:\\s|_))(.+)",1);
////        setArtistRegEx("(?<=(\\s|_)-(\\s|_))(.*?)(?=(\\s|_)-(\\s|_))", 0);
//        break;
//    case SourceDir::ATS:
////        setArtistRegEx(".+?(?=(\\s|_)-(\\s|_))",0);
////        setTitleRegEx("(?<=(\\s|_)-(\\s|_))(.*?)(?=(\\s|_)-(\\s|_))");
////        setSongIdRegEx("(?:^.+(?:\\s|_)-(?:\\s|_).+(?:\\s|_)-(?:\\s|))(.+)", 1);
//        break;
//    case SourceDir::TAS:
////        setTitleRegEx(".+?(?=(\\s|_)-(\\s|_))",0);
////        setArtistRegEx("(?<=(\\s|_)-(\\s|_))(.*?)(?=(\\s|_)-(\\s|_))");
////        setSongIdRegEx("(?:^.+(?:\\s|_)-(?:\\s|_).+(?:\\s|_)-(?:\\s|))(.+)", 1);
//        break;
//    case SourceDir::AT:
////        setArtistRegEx(".+?(?=(\\s|_)-(\\s|_))");
////        setTitleRegEx("(?<=(\\s|_)-(\\s|_))(.*)");
//        break;
//    case SourceDir::TA:
////        setTitleRegEx(".+?(?=(\\s|_)-(\\s|_))");
////        setArtistRegEx("(?<=(\\s|_)-(\\s|_))(.*)");
//        break;
//    case SourceDir::S_T_A:
////        setSongIdRegEx("^[^_]*_");
////        setTitleRegEx("((?<=_)[^_]*_)");
////        setArtistRegEx("([^_]+(?=_[^_]*(\\r?\\n|$)))");
//        break;
//    case SourceDir::METADATA:
//        m_useMetadata = true;
//        break;
//    case SourceDir::CUSTOM:
//        QSqlQuery query;
//        query.exec("SELECT custompattern FROM sourcedirs WHERE m_path == \"" + m_path + "\"" );
//        if (query.first())
//        {
//            customPatternId = query.value(0).toInt();
//        }
//        if (customPatternId < 1)
//        {
//            qCritical() << "Custom pattern set for m_path, but pattern ID is invalid!  Bailing out!";
//            return;
//        }
//        query.exec("SELECT * FROM custompatterns WHERE patternid == " + QString::number(customPatternId));
//        if (query.first())
//        {
//            setArtistRegEx(query.value("artistregex").toString(), query.value("artistcapturegrp").toInt());
//            setTitleRegEx(query.value("titleregex").toString(), query.value("titlecapturegrp").toInt());
//            setSongIdRegEx(query.value("discidregex").toString(), query.value("discidcapturegrp").toInt());
//        }
//        break;
//    }
}

const QString &KaraokeFileInfo::getArtist() {
    getMetadata();
    return m_artist;
//    if (m_useMetadata)
//    {
//        readTags();
//        return m_tagArtist;
//    }
//    QRegularExpression r(m_artistPattern);
//    QRegularExpressionMatch match = r.match(m_fileBaseName);
//    QString result = match.captured(m_artistCaptureGroup);
//    result.replace("_", " ");
//    return result;
}

const QString &KaraokeFileInfo::getTitle() {
    getMetadata();
    return m_title;
//    if (m_useMetadata)
//    {
//        readTags();
//        return m_tagTitle;
//    }
//    QRegularExpression r(m_titlePattern);
//    QRegularExpressionMatch match = r.match(m_fileBaseName);
//    QString result = match.captured(m_titleCaptureGroup);
//    result.replace("_", " ");
//    return result;
}

const QString &KaraokeFileInfo::getSongId() {
    getMetadata();
    return m_songId;
}

QString KaraokeFileInfo::testPattern(const QString &regex, const QString &filename, int captureGroup) {
    QRegularExpression r;
    QRegularExpressionMatch match;
    r.setPattern(regex);
    match = r.match(filename);
    return match.captured(captureGroup).replace("_", " ");

}

uint64_t KaraokeFileInfo::getDuration() {
    if (m_duration > 0)
        return m_duration;
    if (m_fileName.endsWith(".zip", Qt::CaseInsensitive)) {
        MzArchive archive;
        archive.setArchiveFile(m_fileName);
        m_duration = archive.getSongDuration();
        m_crc32 = archive.cdgCRC32();
    } else if (m_fileName.endsWith(".cdg", Qt::CaseInsensitive)) {
        m_duration = ((QFile(m_fileName).size() / 96) / 75) * 1000;
    } else {
        TagReader reader;
        reader.setMedia(m_fileName);
        try {
            m_duration = reader.getDuration();
        }
        catch (...) {
            qInfo() << "KaraokeFileInfo unable to get duration for file: " << m_fileBaseName;
        }
    }
    return m_duration;
}

void KaraokeFileInfo::getMetadata() {
    if (m_artist != "" || m_title != "" || m_songId != "")
        return;
    int customPatternId = 0;
    QString baseNameFiltered = m_fileBaseName;
    baseNameFiltered.replace("_", " ");
    QStringList parts = baseNameFiltered.split(" - ");
    switch (m_pattern) {
        case STA:
            if (!parts.empty())
                m_songId = parts.at(0);
            if (parts.size() >= 2)
                m_title = parts.at(1);
            if (!parts.isEmpty())
                parts.removeFirst();
            if (!parts.isEmpty())
                parts.removeFirst();
            m_artist = parts.join(" - ");
            break;
        case SAT:
            if (!parts.empty())
                m_songId = parts.at(0);
            if (parts.size() >= 2)
                m_artist = parts.at(1);
            if (!parts.isEmpty())
                parts.removeFirst();
            if (!parts.isEmpty())
                parts.removeFirst();
            m_title = parts.join(" - ");
            break;
        case ATS:
            if (!parts.empty())
                m_artist = parts.at(0);
            if (parts.size() >= 3) {
                m_songId = parts.at(parts.size() - 1);
                parts.removeLast();
            }
            if (!parts.isEmpty())
                parts.removeFirst();
            m_title = parts.join(" - ");
            break;
        case TAS:
            if (!parts.empty())
                m_title = parts.at(0);
            if (parts.size() >= 3) {
                m_songId = parts.at(parts.size() - 1);
                if (!parts.isEmpty())
                    parts.removeLast();
            }
            if (!parts.isEmpty())
                parts.removeFirst();
            m_artist = parts.join(" - ");
            break;
        case AT:
            if (!parts.empty()) {
                m_artist = parts.at(0);
                parts.removeFirst();
            }
            m_title = parts.join(" - ");
            break;
        case TA:
            if (!parts.empty()) {
                m_title = parts.at(0);
                parts.removeFirst();
            }
            m_artist = parts.join(" - ");
            break;
        case S_T_A:
            parts = m_fileBaseName.split("_");
            if (!parts.empty())
                m_songId = parts.at(0);
            if (parts.size() >= 2)
                m_title = parts.at(1);
            if (!parts.isEmpty())
                parts.removeFirst();
            if (!parts.isEmpty())
                parts.removeFirst();
            m_artist = parts.join(" - ");
            break;
        case METADATA:
            qInfo() << "Using metadata";
            readTags();
            m_artist = m_tagArtist;
            m_title = m_tagTitle;
            m_songId = m_tagSongid;
            break;
        case CUSTOM:
            QSqlQuery query;
            query.exec("SELECT custompattern FROM sourcedirs WHERE m_path == \"" + m_path + "\"");
            if (query.first()) {
                customPatternId = query.value(0).toInt();
            }
            if (customPatternId < 1) {
                qCritical() << "Custom pattern set for m_path, but pattern ID is invalid!  Bailing out!";
                return;
            }
            query.exec("SELECT * FROM custompatterns WHERE patternid == " + QString::number(customPatternId));
            if (query.first()) {
                setArtistRegEx(query.value("artistregex").toString(), query.value("artistcapturegrp").toInt());
                setTitleRegEx(query.value("titleregex").toString(), query.value("titlecapturegrp").toInt());
                setSongIdRegEx(query.value("discidregex").toString(), query.value("discidcapturegrp").toInt());
            }
            QRegularExpression r;
            QRegularExpressionMatch match;
            r.setPattern(m_artistPattern);
            match = r.match(m_fileBaseName);
            m_artist = match.captured(m_artistCaptureGroup).replace("_", " ");
            r.setPattern(m_titlePattern);
            match = r.match(m_fileBaseName);
            m_title = match.captured(m_titleCaptureGroup).replace("_", " ");
            r.setPattern(m_songIdPattern);
            match = r.match(m_fileBaseName);
            m_songId = match.captured(m_songIdCaptureGroup).replace("_", " ");
            break;
    }
}

uint32_t KaraokeFileInfo::getCRC32checksum() {
    if (m_crc32 > 0)
        return m_crc32;
    if (m_fileName.endsWith(".zip", Qt::CaseInsensitive)) {
        MzArchive archive;
        archive.setArchiveFile(m_fileName);
        m_duration = archive.getSongDuration();
        m_crc32 = archive.cdgCRC32();
    } else if (m_fileName.endsWith(".cdg", Qt::CaseInsensitive)) {
        m_crc32 = crcCalculator.calculateFromFile(m_fileName);
        m_duration = ((QFile(m_fileName).size() / 96) / 75) * 1000;
    } else {
        TagReader reader;
        reader.setMedia(m_fileName);
        try {
            m_duration = reader.getDuration();
        }
        catch (...) {
            qInfo() << "KaraokeFileInfo unable to get duration for file: " << m_fileBaseName;
        }
        m_crc32 = crcCalculator.calculateFromFile(m_fileName);
    }
    return m_crc32;
}

Crc32::Crc32() {
    quint32 crc;

    // initialize CRC table
    for (int i = 0; i < 256; i++) {
        crc = i;
        for (int j = 0; j < 8; j++)
            crc = crc & 1 ? (crc >> 1) ^ 0xEDB88320UL : crc >> 1;

        crc_table[i] = crc;
    }
}

quint32 Crc32::calculateFromFile(const QString &filename) {
    quint32 crc;
    QFile file;

    char buffer[16000];
    uint64_t len;
    int i;

    crc = 0xFFFFFFFFUL;

    file.setFileName(filename);
    if (file.open(QIODevice::ReadOnly)) {
        while (!file.atEnd()) {
            len = file.read(buffer, 16000);
            for (i = 0; i < len; i++)
                crc = crc_table[(crc ^ buffer[i]) & 0xFF] ^ (crc >> 8);
        }

        file.close();
    }

    return crc ^ 0xFFFFFFFFUL;
}

void Crc32::initInstance(int i) {
    instances[i] = 0xFFFFFFFFUL;
}

void Crc32::pushData(int i, const char *data, int len) {
    quint32 crc = instances[i];
    if (crc) {
        for (int j = 0; j < len; j++)
            crc = crc_table[(crc ^ data[j]) & 0xFF] ^ (crc >> 8);

        instances[i] = crc;
    }
}

quint32 Crc32::releaseInstance(int i) {
    quint32 crc32 = instances[i];
    if (crc32) {
        instances.remove(i);
        return crc32 ^ 0xFFFFFFFFUL;
    } else {
        return 0;
    }
}
