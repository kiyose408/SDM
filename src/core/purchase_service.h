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

    /// 获取今日采购清单（扣除冰箱已有库存后的需购量）
    Q_INVOKABLE QVariantList getTodayPurchaseList(const QString &familyId, const QString &date);

    /// 确认采购并入库（每项可自定义实际购买量）
    Q_INVOKABLE bool confirmPurchase(const QString &familyId, const QString &userId,
                                      const QVariantList &items);

private:
    DailyMenuRepository *dmRepo_;
    MenuItemRepository *miRepo_;
    RecipeRepository *recipeRepo_;
    RecipeIngredientRepository *riRepo_;
    IngredientRepository *ingRepo_;
};

} // namespace smart_diet
