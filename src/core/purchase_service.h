#pragma once
#include <QObject>
#include <QVariantList>
#include <QVariantMap>
#include "base_service.h"

namespace smart_diet {
class DailyMenuRepository;
class MenuItemRepository;
class RecipeRepository;
class RecipeIngredientRepository;
class IngredientRepository;

class PurchaseService : public BaseService {
    Q_OBJECT
public:
    explicit PurchaseService(DailyMenuRepository *dm, MenuItemRepository *mi,
                             RecipeRepository *rr, RecipeIngredientRepository *ri,
                             IngredientRepository *ir, QObject *parent = nullptr);

    /// 获取今日采购清单（按食材聚合，含分类和总量）
    Q_INVOKABLE QVariantList getTodayPurchaseList(const QString &familyId, const QString &date);

private:
    DailyMenuRepository *dmRepo_;
    MenuItemRepository *miRepo_;
    RecipeRepository *recipeRepo_;
    RecipeIngredientRepository *riRepo_;
    IngredientRepository *ingRepo_;
};

} // namespace smart_diet
