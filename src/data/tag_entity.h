#pragma once
#include <QString>

namespace smart_diet {

struct Tag {
    QString id;
    QString name;       // 标签名（如"高蛋白"、"快手菜"）
    QString type;       // flavor / cooking_method / occasion / nutrition
    QString created_at;
};

} // namespace smart_diet
