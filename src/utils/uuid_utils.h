#pragma once

#include <QUuid>
#include <QString>

namespace smart_diet {

/**
 * @brief UUID 工具 — 生成符合项目规范的主键
 *
 * 规范来源（004.数据库设计文档 §二）：
 * - 主键统一为 UUID v4 字符串
 * - 格式：36 位，无连字符（与 QUuid::WithoutBraces 一致）
 * - 示例：d4a2f7b1c9e34f8a9b6c1d2e3f4a5b6c
 */
inline QString generateUuid() {
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

} // namespace smart_diet
