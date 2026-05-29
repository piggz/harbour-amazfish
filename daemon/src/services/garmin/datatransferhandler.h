#ifndef DATATRANSFERHANDLER_H
#define DATATRANSFERHANDLER_H

#include <QtCore/QByteArray>
#include <QtCore/QDateTime>
#include <QtCore/QHash>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtCore/QSharedPointer>
#include <QtCore/QtGlobal>

#include <atomic>
#include <optional>

/// Maximum number of active transfers to keep in memory
static constexpr int MAX_ACTIVE_TRANSFERS = 100;

/// Time to keep transfers in registry before auto-cleanup (5 minutes)
static constexpr qint64 TRANSFER_TIMEOUT_MS = 300000;

/// Statistics about the data transfer registry
struct TransferStats {
    int activeTransfers = 0;
    qint64 totalBytes = 0;
    std::optional<qint64> oldestTransferAgeMs; // milliseconds
};

/// Transfer metadata
struct TransferInfo {
    QByteArray data;
    qint64 createdAtMs = 0;
    qint64 lastAccessedMs = 0;
};
class DataTransferHandler
{
public:
    DataTransferHandler();

    /// Register data for transfer and return a unique transfer ID
    quint32 registerData(const QByteArray& data);

    /// Get the total size of a registered transfer
    std::optional<int> getSize(quint32 id);

    /// Get a chunk of data from a registered transfer
    std::optional<QByteArray> getChunk(quint32 id, int offset, int length);

    /// Release a transfer from the registry
    bool release(quint32 id);

    /// Check if a transfer exists
    bool exists(quint32 id) const;

    /// Get the number of active transfers
    int activeCount() const;

    /// Cleanup old transfers that haven't been accessed recently
    void cleanupOldTransfers();

    /// Get statistics about the data transfer registry
    TransferStats getStats() const;

private:
    void cleanupOldTransfersLocked(QHash<quint32, TransferInfo>& registry);

    static qint64 nowMs();

private:
    mutable QMutex m_mutex;
    QHash<quint32, TransferInfo> m_registry;
    std::atomic<quint32> m_nextId;
};


#endif // DATATRANSFERHANDLER_H
