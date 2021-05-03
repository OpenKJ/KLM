#ifndef KARAOKEFILE_H
#define KARAOKEFILE_H
#include <QString>
#include <vector>

class KaraokeFile
{
private:
    QString m_artist;
    QString m_title;
    QString m_songid;
    QString m_path;
    uint64_t m_duration{0};
    uint32_t m_crc32{0};
    bool m_fileScanned{false};


public:
    [[nodiscard]] QString artist() const { return m_artist; }
    [[nodiscard]] QString title() const { return m_title; }
    [[nodiscard]] QString songid() const { return m_songid; }
    QString path() const { return m_path; }
    void setPath(const QString &path);
    uint32_t crc();
    uint64_t duration();
    void scanFile();
    uint getBitrate();
};

typedef std::vector<KaraokeFile> KaraokeFiles;

#endif // KARAOKEFILE_H
