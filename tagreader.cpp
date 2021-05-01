#include "tagreader.h"
#include <QDebug>
#include <tag.h>
#include <taglib/fileref.h>

TagReader::TagReader(QObject *parent) : QObject(parent) {
    m_duration = 0;

}

TagReader::~TagReader() {
}

QString TagReader::getArtist() {
    return m_artist;
}

QString TagReader::getTitle() {
    return m_title;
}

QString TagReader::getAlbum() {
    return m_album;
}

QString TagReader::getTrack() {
    return m_track;
}

unsigned int TagReader::getDuration() {
    return m_duration;
}

void TagReader::setMedia(QString path) {
    qInfo() << "Getting tags for: " << path;
    if ((path.endsWith(".mp3", Qt::CaseInsensitive)) || (path.endsWith(".ogg", Qt::CaseInsensitive)) ||
        path.endsWith(".mp4", Qt::CaseInsensitive) || path.endsWith(".m4v", Qt::CaseInsensitive)) {
        qInfo() << "Using taglib to get tags";
        taglibTags(path);
        return;
    }
}

void TagReader::taglibTags(QString path) {
    TagLib::FileRef f(path.toLocal8Bit().data());
    if (!f.isNull()) {
        m_artist = f.tag()->artist().toCString(true);
        m_title = f.tag()->title().toCString(true);
        m_duration = f.audioProperties()->length() * 1000;
        m_album = f.tag()->album().toCString(true);
        int track = f.tag()->track();
        if (track == 0)
            m_track = QString();
        else if (track < 10)
            m_track = "0" + QString::number(track);
        else
            m_track = QString::number(track);
        qInfo() << "Taglib result - Artist: " << m_artist << " Title: " << m_title << " Album: " << m_album
                << " Track: " << m_track << " Duration: " << m_duration;
    } else {
        qWarning() << "Taglib was unable to process the file";
        m_artist = QString();
        m_title = QString();
        m_album = QString();
        m_duration = 0;
    }
}
