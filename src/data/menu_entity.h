#pragma once
#include <QString>

namespace smart_diet {

struct DailyMenu {
    QString id;
    QString family_id;
    QString date;           // YYYY-MM-DD
    QString meal_type;      // breakfast / lunch / dinner
    int     diner_count = 1;
    QString diet_mode = "maintenance";
    bool    is_confirmed = false;
    QString confirmed_by;
    QString confirmed_at;
    QString chef_id;
    bool    is_deleted = false;
    int     version = 1;
    QString created_at;
    QString updated_at;
};

struct MenuItem {
    QString id;
    QString menu_id;
    QString recipe_id;
    bool    is_locked = false;
    int     servings_override = 0;  // 0=用默认
    QString status = "pending";     // pending / cooking / completed
    int     sort_order = 0;
    int     version = 1;
    QString created_at;
    QString updated_at;
};

} // namespace smart_diet
