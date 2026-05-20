#pragma once

#include <QSqlQuery>
#include <QString>
#include <QVariant>
#include <optional>

namespace smart_diet {

/**
 * @brief 轻量 ORM 辅助工具
 *
 * 提供 Repository 层常用的参数绑定和结果读取辅助函数。
 * 目的：减少每个 Repository 中的 SQL 样板代码，统一参数命名规范。
 *
 * 不是完整的 ORM 框架 —— 每个 Repository 自己写 SQL，
 * 此类只提供可复用的绑定/读取片段。
 */
class OrmHelper {
public:
    // ================================================================
    // 参数绑定（约定命名规范，减少手写 :param 出错）
    // ================================================================

    /// 绑定 :id
    static void bindId(QSqlQuery &query, const QString &uuid) {
        query.bindValue(QStringLiteral(":id"), uuid);
    }

    /// 绑定 :id + :version（乐观锁 UPDATE 的 WHERE 子句）
    static void bindIdAndVersion(QSqlQuery &query,
                                  const QString &uuid, int version) {
        query.bindValue(QStringLiteral(":id"), uuid);
        query.bindValue(QStringLiteral(":version"), version);
    }

    /// 绑定 :is_deleted（软删除标记）
    static void bindSoftDeleted(QSqlQuery &query, bool isDeleted) {
        query.bindValue(QStringLiteral(":is_deleted"), isDeleted ? 1 : 0);
    }

    // ================================================================
    // 结果读取（约定字段名，减少手写 column index）
    // ================================================================

    /// 读取 TEXT 字段，为 NULL 时返回空字符串
    static QString readString(const QSqlQuery &query, const QString &column) {
        const QVariant v = query.value(column);
        return v.isNull() ? QString() : v.toString();
    }

    /// 读取 INTEGER (bool) 字段，为 NULL 时返回 0
    static int readBool(const QSqlQuery &query, const QString &column) {
        const QVariant v = query.value(column);
        return v.isNull() ? 0 : v.toInt();
    }

    /// 读取 REAL 字段，为 NULL 时返回 0.0
    static double readReal(const QSqlQuery &query, const QString &column) {
        const QVariant v = query.value(column);
        return v.isNull() ? 0.0 : v.toDouble();
    }

    /// 读取 INTEGER 字段，为 NULL 时返回 0
    static int readInt(const QSqlQuery &query, const QString &column) {
        const QVariant v = query.value(column);
        return v.isNull() ? 0 : v.toInt();
    }

    /// 尝试读取单个 TEXT 值，可能为空（用于 SELECT 单值的场景）
    static std::optional<QString> readOptionalString(const QSqlQuery &query,
                                                      const QString &column) {
        const QVariant v = query.value(column);
        if (v.isNull()) return std::nullopt;
        return v.toString();
    }

    // ================================================================
    // 结果存在性检查
    // ================================================================

    /// 检查最后执行的查询是否有返回行
    static bool hasRow(const QSqlQuery &query) {
        return query.isActive() && query.isSelect() && query.size() > 0;
    }
};

} // namespace smart_diet
