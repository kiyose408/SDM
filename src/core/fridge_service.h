#pragma once
#include <QObject>
#include <QVariantList>
#include <QVariantMap>
#include "base_service.h"

namespace smart_diet {
class InventoryBatchRepository;
class IngredientRepository;

class FridgeService : public BaseService {
    Q_OBJECT
public:
    explicit FridgeService(InventoryBatchRepository *ib, IngredientRepository *ir, QObject *parent = nullptr);

    /// 获取家庭冰箱库存列表
    Q_INVOKABLE QVariantList getStock(const QString &familyId);

    /// 手动添加食材到冰箱
    Q_INVOKABLE bool addItem(const QString &familyId, const QString &ingredientId,
                              double quantity, const QString &unit,
                              const QString &expiryDate, const QString &addedBy);

    /// 消耗食材（核销指定批次）
    Q_INVOKABLE bool deductBatch(const QString &batchId, double amount, const QString &operatorId, const QString &reason);

    /// 按食材 ID 消耗（先进先出 FIFO）
    Q_INVOKABLE bool deductByFifo(const QString &familyId, const QString &ingredientId,
                                   double totalNeeded, const QString &operatorId, const QString &reason);

    /// 采购清单一键入库
    Q_INVOKABLE int importFromPurchase(const QString &familyId, const QString &addedBy);

    /// 校准库存：将指定食材总量设为 newTotal（清旧批次→建新批次）
    Q_INVOKABLE bool calibrateStock(const QString &familyId, const QString &ingredientId,
                                     double newTotal, const QString &operatorId);

private:
    InventoryBatchRepository *ibRepo_;
    IngredientRepository *ingRepo_;
};

} // namespace smart_diet
