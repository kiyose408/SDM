#pragma once
#include <QObject>
#include <QVariantList>
#include <QVariantMap>
#include "base_service.h"

namespace smart_diet {
class DailyMenuRepository;
class MenuItemRepository;
class RecipeRepository;
class IngredientRepository;

class MenuService : public BaseService {
    Q_OBJECT
public:
    explicit MenuService(DailyMenuRepository *dm, MenuItemRepository *mi,
                         RecipeRepository *rr, QObject *parent = nullptr);

    /// 为指定家庭和日期生成/获取今日菜单（三时段）
    Q_INVOKABLE QVariantMap getTodayMenu(const QString &familyId, const QString &date);

    /// 生成推荐菜谱并填入菜单
    Q_INVOKABLE QVariantMap generateMenu(const QString &familyId, const QString &date,
                                          const QString &mealType, int count = 2,
                                          const QString &dietMode = QStringLiteral("maintenance"),
                                          int dinerCount = 2);

    /// 添加菜品到菜单
    Q_INVOKABLE bool addDish(const QString &menuId, const QString &recipeId);

    /// 移除菜品
    Q_INVOKABLE bool removeDish(const QString &menuId, const QString &recipeId);

    /// 换一换（替换菜品）
    Q_INVOKABLE bool swapDish(const QString &menuId, const QString &oldRecipeId);

    /// 锁定/解锁菜品
    Q_INVOKABLE bool toggleLock(const QString &menuId, const QString &recipeId);

    /// 调整菜品份数
    Q_INVOKABLE bool adjustServings(const QString &menuId, const QString &recipeId, double servings);

    /// 获取今日营养摘要（当前摄入 + 推荐目标）
    Q_INVOKABLE QVariantMap getNutritionSummary(const QString &familyId, const QString &date,
                                                  const QString &dietMode, int dinerCount,
                                                  double userTdee = 2000);

signals:
    void menuGenerated(const QString &familyId, const QString &date);

private:
    QVariantMap menuToMap(const struct DailyMenu &m, const QList<struct MenuItem> &items);
    QString findOrCreateMenu(const QString &familyId, const QString &date, const QString &mealType);

    DailyMenuRepository *dmRepo_;
    MenuItemRepository *miRepo_;
    RecipeRepository *recipeRepo_;
};

} // namespace smart_diet
