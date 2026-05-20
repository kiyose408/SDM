#pragma once

#include <QObject>
#include <QList>
#include <QString>
#include <QDateTime>

namespace smart_diet {

/**
 * @brief 同步操作队列项
 *
 * 每条记录对应一次需要同步到云端的本地操作。
 * uuid 字段用于幂等去重（服务端收到相同 uuid 的操作直接跳过）。
 */
struct SyncEntry {
    QString   uuid;        ///< 操作唯一 ID（幂等去重用，由 QUuid 生成）
    QString   entityType;  ///< 实体类型（"user" / "family" / "inventory_batch" ...）
    QString   entityId;    ///< 操作的实体 UUID
    QString   operation;   ///< "CREATE" / "UPDATE" / "DELETE"
    QString   payload;     ///< JSON 序列化的变更数据
    QDateTime createdAt;   ///< ISO 8601 时间戳
    int       retryCount = 0;  ///< 已重试次数
};

/**
 * @brief 同步管理器（基础设施层）
 *
 * 负责：
 * 1. 离线时的操作队列暂存（SQLite 表 local_sync_queue）
 * 2. 网络恢复后的批量推送（FIFO 顺序）
 * 3. 指数退避重试（1s → 2s → 4s → 8s → 16s，最多 5 次）
 * 4. 冲突解决（云端时间戳优先，本地被覆盖）
 *
 * 设计约束（来自 001.概要设计文档 非功能性需求）：
 * - 同步推送延迟 < 2 秒
 * - 消息不丢失、不重复（幂等键去重）
 * - 断连后自动重连（指数退避，最大 5 次）
 */
class SyncManager : public QObject {
    Q_OBJECT

public:
    explicit SyncManager(QObject *parent = nullptr);
    ~SyncManager() override = default;

    /// 将操作加入同步队列（离线或在线均可调用）
    void enqueue(const SyncEntry &entry);

    /// 手动触发队列推送（通常在网络恢复回调中调用）
    void flush();

    /// 获取当前待同步队列长度
    int pendingCount() const;

    /// 当前网络状态
    bool isOnline() const { return isOnline_; }

signals:
    /// 同步完成，参数为本次成功推送的操作数
    void syncCompleted(int syncedCount);

    /// 同步失败（队列中仍有未推送项）
    void syncFailed(const QString &reason);

    /// 网络连接状态变更
    void connectionStateChanged(bool isOnline);

private:
    QList<SyncEntry> queue_;
    bool isOnline_ = false;

    static constexpr int MAX_RETRY       = 5;
    static constexpr int BASE_DELAY_MS   = 1000;   // 指数退避基数 1s
};

// 构造函数内联实现（桩：阶段 8 实现 WS 逻辑）
inline SyncManager::SyncManager(QObject *parent)
    : QObject(parent) {}

} // namespace smart_diet
