#pragma once
#include <QObject>
#include <QVariantList>
#include <QVariantMap>
#include "base_service.h"

namespace smart_diet {
class MenuItemRepository;
class RecipeRepository;
class RecipeIngredientRepository;
class IngredientRepository;
class InventoryBatchRepository;

class ConsumptionService : public BaseService {
    Q_OBJECT
public:
    explicit ConsumptionService(MenuItemRepository *mi, RecipeRepository *rr,
                                 RecipeIngredientRepository *ri, IngredientRepository *ir,
                                 InventoryBatchRepository *ib, QObject *parent = nullptr);

    /// 标记一道菜"已完成消耗"→扣除冰箱食材
    Q_INVOKABLE bool consumeRecipe(const QString &menuItemId, const QString &familyId,
                                    double servings, const QString &operatorId);
    /// 撤销消耗→回补冰箱+恢复状态为 pending
    Q_INVOKABLE bool undoConsume(const QString &menuItemId, const QString &operatorId);

    /// 获取家庭消耗历史
    Q_INVOKABLE QVariantList getConsumptionHistory(const QString &familyId, int limit = 50);

private:
    MenuItemRepository *miRepo_;
    RecipeRepository *recipeRepo_;
    RecipeIngredientRepository *riRepo_;
    IngredientRepository *ingRepo_;
    InventoryBatchRepository *ibRepo_;
};

} // namespace smart_diet
