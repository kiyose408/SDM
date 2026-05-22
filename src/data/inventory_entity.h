#pragma once
#include <QString>

namespace smart_diet {

struct InventoryBatch {
    QString id;
    QString family_id;
    QString ingredient_id;
    double  batch_quantity = 0;
    QString unit;
    QString expiry_date;
    QString purchase_date;
    QString source;          // manual_add / purchase_list_batch / menu_deduction
    QString added_by;        // user id
    bool    is_deleted  = false;
    int     version     = 1;
    QString created_at;
    QString updated_at;

    // 关联展示用（非表字段，由 JOIN 填充）
    QString ingredient_name;
    QString ingredient_category;
    double  days_until_expiry = 9999;
};

} // namespace smart_diet
