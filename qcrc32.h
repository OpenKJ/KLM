#ifndef KLM_QCRC32_H
#define KLM_QCRC32_H


#include <QtCore>
#include <QString>
#include <QMap>

class Crc32
{
private:
    quint32 crc_table[256]{};
    QMap<int, quint32> instances;

public:
    Crc32();

    quint32 calculateFromFile(const QString& filename);
    void initInstance(int i);
    void pushData(int i, const char *data, int len);
    quint32 releaseInstance(int i);
};


#endif //KLM_QCRC32_H