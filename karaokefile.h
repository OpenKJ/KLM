#ifndef KARAOKEFILE_H
#define KARAOKEFILE_H
#include <QString>
#include <vector>
#include "karaokefileinfo.h"

class KaraokeFile
{
private:
    QString m_artist;
    QString m_title;
    QString m_songid;
    QString m_path;
    uint64_t m_duration{0};
    uint32_t m_crc32{0};
    KaraokeFileInfo m_info;
    bool m_crc_check_run{false};
public:
    QString artist() const { return m_artist; }
    void setArtist(const QString &artist);
    QString title() const { return m_title; }
    void setTitle(const QString &title);
    QString songid() const { return m_songid; }
    void setSongid(const QString &songid);
    QString path() const { return m_path; }
    void setPath(const QString &path);
    uint32_t crc();
    uint64_t duration();
};

typedef std::vector<KaraokeFile> KaraokeFiles;

#endif // KARAOKEFILE_H
