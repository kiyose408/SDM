#pragma once

#include <QString>

namespace smart_diet {

/**
 * @brief Argon2id 密码哈希器
 *
 * 算法参数（来自 005.接口设计文档 §3.1 安全设计文档 v1.0）：
 * - 内存消耗: 64 MB (m_cost = 65536)
 * - 迭代次数: 3  (t_cost = 3)
 * - 并行度:   4  (p_cost = 4)
 * - 盐长度:   16 bytes (随机生成)
 * - 哈希长度: 32 bytes
 *
 * 输出格式: $argon2id$v=19$m=65536,t=3,p=4$<base64_salt>$<base64_hash>
 */
class Argon2Hasher {
public:
    /**
     * @brief 哈希明文密码
     * @param plainText 明文密码
     * @return Argon2id 哈希字符串，失败返回空
     */
    static QString hash(const QString &plainText);

    /**
     * @brief 验证密码
     * @param plainText  明文密码
     * @param encodedHash hash() 输出的编码哈希
     * @return 是否匹配
     */
    static bool verify(const QString &plainText, const QString &encodedHash);
};

} // namespace smart_diet
