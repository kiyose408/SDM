#pragma once

#include <QDateTime>
#include <QString>

namespace smart_diet {

/**
 * @brief 时间工具 — 生成符合项目规范的 ISO 8601 时间戳
 *
 * 规范来源（004.数据库设计文档 §二）：
 * - 格式：ISO 8601 文本（2026-05-18T12:00:00Z）
 * - 时区：统一使用 UTC
 */
inline QString utcNow() {
    return QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
}

} // namespace smart_diet
