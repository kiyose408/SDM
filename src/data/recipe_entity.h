#pragma once

#include <QString>

namespace smart_diet {

/**
 * @brief 菜谱实体（对应 recipes 表）
 */
struct Recipe {
    QString id;
    QString name;
    QString cover_image_url;
    QString description;
    int     cooking_time    = 0;   // 分钟
    int     servings        = 2;
    QString meal_type       = QStringLiteral("any");
    double  total_calories  = 0.0;
    double  total_protein   = 0.0;
    double  total_carbs     = 0.0;
    double  total_fat       = 0.0;
    bool    is_system       = true;
    QString creator_id;
    QString family_id;
    int     popularity      = 0;
    bool    is_deleted      = false;
    int     version         = 1;
    QString created_at;
    QString updated_at;
};

/**
 * @brief 菜谱食材关联实体（对应 recipe_ingredients 表）
 */
struct RecipeIngredient {
    QString id;
    QString recipe_id;
    QString ingredient_id;
    double  amount          = 0.0;
    QString created_at;
};

} // namespace smart_diet
