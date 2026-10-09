#include "datatransferhandler.h"

#include <QtCore/QDebug>

qint64 DataTransferHandler::nowMs()
{
    return QDateTime::currentMSecsSinceEpoch();
}

DataTransferHandler::DataTransferHandler()
    : m_nextId(1) // Start at 1, 0 is invalid
{
}


quint32 DataTransferHandler::registerData(const QByteArray& data)
{
    const quint32 id = m_nextId.fetch_add(1, std::memory_order_seq_cst);

    const qint64 now = nowMs();
    TransferInfo info;
    info.data = data;
    info.createdAtMs = now;
    info.lastAccessedMs = now;

    QMutexLocker lock(&m_mutex);

    // Cleanup old transfers if we're at the limit
    if (m_registry.size() >= MAX_ACTIVE_TRANSFERS) {
        cleanupOldTransfersLocked(m_registry);
    }

    m_registry.insert(id, info);

    qInfo() << "Registered data transfer: id =" << id
            << ", size =" << m_registry.value(id).data.size() << "bytes";

    return id;
}

std::optional<int> DataTransferHandler::getSize(quint32 id)
{
    QMutexLocker lock(&m_mutex);

    auto it = m_registry.find(id);
    if (it == m_registry.end()) {
        return std::nullopt;
    }

    it->lastAccessedMs = nowMs();
    return it->data.size();
}

std::optional<QByteArray> DataTransferHandler::getChunk(quint32 id, int offset, int length)
{
    QMutexLocker lock(&m_mutex);

    auto it = m_registry.find(id);
    if (it == m_registry.end()) {
        qWarning() << "Transfer" << id << "not found in registry";
        return std::nullopt;
    }

    it->lastAccessedMs = nowMs();

    if (offset < 0 || offset >= it->data.size()) {
        qWarning() << "Transfer" << id
                   << ": offset" << offset
                   << "out of bounds (size:" << it->data.size() << ")";
        return std::nullopt;
    }

    const int end = qMin(offset + length, it->data.size());
    const QByteArray chunk = it->data.mid(offset, end - offset);

    qDebug() << "Transfer" << id
             << ": serving chunk at offset" << offset
             << "(requested:" << length
             << ", actual:" << chunk.size() << "bytes)";

    return chunk;
}

bool DataTransferHandler::release(quint32 id)
{
    QMutexLocker lock(&m_mutex);

    auto it = m_registry.find(id);
    if (it == m_registry.end()) {
        qWarning() << "Attempted to release non-existent transfer: id =" << id;
        return false;
    }

    const int size = it->data.size();
    m_registry.erase(it);

    qInfo() << "Released data transfer: id =" << id
            << ", size =" << size << "bytes";

    return true;
}

bool DataTransferHandler::exists(quint32 id) const
{
    QMutexLocker lock(&m_mutex);
    return m_registry.contains(id);
}

int DataTransferHandler::activeCount() const
{
    QMutexLocker lock(&m_mutex);
    return m_registry.size();
}

void DataTransferHandler::cleanupOldTransfers()
{
    QMutexLocker lock(&m_mutex);
    cleanupOldTransfersLocked(m_registry);
}

void DataTransferHandler::cleanupOldTransfersLocked(QHash<quint32, TransferInfo>& registry)
{
    const qint64 now = nowMs();
    QVector<quint32> toRemove;
    toRemove.reserve(registry.size());

    for (auto it = registry.begin(); it != registry.end(); ++it) {
        const qint64 ageSinceLastAccess = now - it->lastAccessedMs;
        if (ageSinceLastAccess > TRANSFER_TIMEOUT_MS) {
            toRemove.push_back(it.key());
        }
    }

    for (quint32 id : toRemove) {
        auto it = registry.find(id);
        if (it == registry.end()) {
            continue;
        }

        const qint64 ageSinceCreate = now - it->createdAtMs;
        const int size = it->data.size();

        registry.erase(it);

        qInfo() << "Cleaned up stale transfer: id =" << id
                << ", size =" << size << "bytes"
                << ", age =" << ageSinceCreate << "ms";
    }
}

TransferStats DataTransferHandler::getStats() const
{
    QMutexLocker lock(&m_mutex);

    const qint64 now = nowMs();

    TransferStats stats;
    stats.activeTransfers = m_registry.size();
    stats.totalBytes = 0;
    stats.oldestTransferAgeMs = std::nullopt;

    for (auto it = m_registry.begin(); it != m_registry.end(); ++it) {
        const TransferInfo& info = it.value();

        stats.totalBytes += info.data.size();

        const qint64 age = now - info.createdAtMs;
        if (stats.oldestTransferAgeMs.has_value()) {
            if (age > *stats.oldestTransferAgeMs) {
                stats.oldestTransferAgeMs = age;
            }
        } else {
            stats.oldestTransferAgeMs = age;
        }
    }

    return stats;
}
