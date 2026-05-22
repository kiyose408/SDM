#pragma once
#include <QObject>
#include <QSqlDatabase>
#include <QList>
#include <optional>
#include "base_repository.h"
#include "inventory_entity.h"

namespace smart_diet {

class InventoryBatchRepository : public QObject, public BaseRepository<InventoryBatch> {
    Q_OBJECT
public:
    explicit InventoryBatchRepository(const QSqlDatabase &db, QObject *parent = nullptr);

    std::optional<InventoryBatch> getById(const QString &id) override;
    QList<InventoryBatch> getAll() override;
    bool save(const InventoryBatch &entity) override;
    bool update(const InventoryBatch &entity) override;
    bool softDelete(const QString &id) override;
    bool updateWithVersion(const InventoryBatch &entity, int expectedVersion) override;

    /// 按家庭 ID + 食材 ID 查询所有未删除批次
    QList<InventoryBatch> getByFamilyAndIngredient(const QString &familyId, const QString &ingredientId);

    /// 获取家庭全部库存（含食材名称/类别 JOIN）
    QList<InventoryBatch> getFamilyInventory(const QString &familyId);

    /// 获取指定批次（含食材名）
    InventoryBatch getBatchWithName(const QString &batchId);

private:
    InventoryBatch mapRow(const QSqlQuery &query);
    QSqlDatabase db_;
};

} // namespace smart_diet
