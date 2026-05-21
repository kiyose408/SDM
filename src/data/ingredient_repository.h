#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QList>
#include <optional>
#include "base_repository.h"
#include "ingredient_entity.h"

namespace smart_diet {

class IngredientRepository : public QObject, public BaseRepository<Ingredient> {
    Q_OBJECT

public:
    explicit IngredientRepository(QObject *parent = nullptr);

    std::optional<Ingredient> getById(const QString &uuid) override;
    QList<Ingredient> getAll() override;
    bool save(const Ingredient &entity) override;
    bool update(const Ingredient &entity) override;
    bool softDelete(const QString &uuid) override;
    bool updateWithVersion(const Ingredient &entity, int expectedVersion) override;

    QList<Ingredient> getByCategory(const QString &category);
    QList<Ingredient> searchByName(const QString &keyword);

signals:
    void ingredientInserted(const QString &id);
    void ingredientUpdated(const QString &id);

private:
    static Ingredient mapRow(const QSqlQuery &query);
    QSqlDatabase db_;
};

} // namespace smart_diet
