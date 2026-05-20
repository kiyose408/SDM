#pragma once

#include <QList>
#include <QString>
#include <optional>

namespace smart_diet {

/**
 * @brief Repository 模式抽象接口（纯虚基类，非 QObject）
 *
 * 所有数据访问层实现必须实现此接口，提供统一的 CRUD + 软删除 + 乐观锁接口。
 * T 为实体类型（如 User、Family、Recipe 等）。
 *
 * ⚠️ 注意：此类不继承 QObject，不使用 Q_OBJECT。
 * 原因：Qt MOC 不支持模板类中的 Q_OBJECT 宏（编译期无法确定元对象结构）。
 * 派生类（如 UserRepository）应采用多重继承，QObject 必须为第一基类：
 *   class UserRepository : public QObject, public BaseRepository<User> { Q_OBJECT ... };
 *
 * 设计决策（来自 004.数据库设计文档）：
 * - 主键统一为 UUID v4 字符串（TEXT, 36 位）
 * - 时间戳统一 ISO 8601 格式
 * - 删除操作统一软删除（is_deleted = 1）
 * - 更新操作须携带版本号进行乐观锁校验
 */
template <typename T>
class BaseRepository {
public:
    virtual ~BaseRepository() = default;

    // ===== 标准 CRUD =====
    virtual std::optional<T> getById(const QString &uuid) = 0;
    virtual QList<T> getAll() = 0;
    virtual bool save(const T &entity) = 0;
    virtual bool update(const T &entity) = 0;

    /**
     * @brief 软删除（设 is_deleted = true）
     * @return 是否成功标记删除
     *
     * 硬删除仅在 sync_queue 推送确认后由同步引擎执行物理清理。
     * 业务层代码一律调用此方法。
     */
    virtual bool softDelete(const QString &uuid) = 0;

    // ===== 乐观锁版本控制 =====
    /**
     * @brief 带版本号校验的更新
     * @param entity          待更新实体（含新值 + 期望版本号）
     * @param expectedVersion 期望的当前版本号
     * @return 是否更新成功（版本不匹配时返回 false）
     *
     * 实现时应使用 SQL:
     *   UPDATE ... SET ... WHERE id = ? AND version = ?
     * 若 affected_rows = 0，说明版本冲突，应触发重试或冲突通知。
     */
    virtual bool updateWithVersion(const T &entity, int expectedVersion) = 0;
};

} // namespace smart_diet
