#ifndef FILENAMEPARSER_H
#define FILENAMEPARSER_H

#include <QObject>
#include "tagreader.h"
#include <QMap>

class Crc32
{
private:
    quint32 crc_table[256]{};
    QMap<int, quint32> instances;

public:
    Crc32();

    quint32 calculateFromFile(const QString &filename);

    void initInstance(int i);
    void pushData(int i, const char *data, int len);
    quint32 releaseInstance(int i);
};

class KaraokeFileInfo : public QObject
{
    Q_OBJECT
    QString m_artistPattern;
    int m_artistCaptureGroup{0};
    QString m_titlePattern;
    int m_titleCaptureGroup{0};
    QString m_songIdPattern;
    int m_songIdCaptureGroup{0};
    QString m_fileName;
    QString m_fileBaseName;
    bool m_tagsRead{false};
    QString m_tagArtist;
    QString m_tagTitle;
    QString m_tagSongid;
    uint64_t m_duration{0};
    uint32_t m_crc32{0};
    int m_pattern{NamingPattern::SAT};
    QString m_path;
    QString m_artist;
    QString m_title;
    QString m_songId;
    Crc32 crcCalculator;
    void readTags();

public:
    enum NamingPattern {SAT=0,STA,ATS,TAS,AT,TA,CUSTOM,METADATA,S_T_A};
    void setArtistRegEx(const QString &pattern, const int captureGroup = 0) { m_artistPattern = pattern; m_artistCaptureGroup = captureGroup;}
    void setTitleRegEx(const QString &pattern, const int captureGroup = 0) { m_titlePattern = pattern; m_titleCaptureGroup = captureGroup;}
    void setSongIdRegEx(const QString &pattern, const int captureGroup = 0) { m_songIdPattern = pattern; m_songIdCaptureGroup = captureGroup;}
    void setFileName(const QString &filename);
    void setPattern(int pattern, const QString &path = "");
    const QString& getArtist();
    const QString& getTitle();
    const QString& getSongId();
    static QString testPattern(const QString &regex, const QString &filename, int captureGroup = 0);
    uint64_t getDuration();
    uint32_t getCRC32checksum();
    void getMetadata();

signals:

public slots:
};

#endif // FILENAMEPARSER_H
