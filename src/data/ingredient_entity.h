#pragma once

#include <QString>

namespace smart_diet {

/**
 * @brief 食材实体（对应 ingredients 表）
 *
 * 字段定义与 004.数据库设计文档 §4.4 严格一致。
 */
struct Ingredient {
    QString id;
    QString name;               // 食材名称
    QString unit;               // g 或 ml
    double  calories_per_100 = 0.0;
    double  protein_per_100  = 0.0;
    double  carbs_per_100    = 0.0;
    double  fat_per_100      = 0.0;
    QString image_url;
    QString category;           // vegetable/meat/seafood/staple/condiment/dairy/other
    bool    is_system     = true;
    QString creator_id;         // 自定义食材的创建者
    int     version       = 1;
    QString created_at;
    QString updated_at;
};

} // namespace smart_diet
