#pragma once
#include <QObject>
#include <QSqlDatabase>
#include <QList>
#include <optional>
#include "menu_entity.h"

namespace smart_diet {

class DailyMenuRepository : public QObject {
    Q_OBJECT
public:
    explicit DailyMenuRepository(QObject *parent = nullptr);

    std::optional<DailyMenu> getById(const QString &id);
    std::optional<DailyMenu> getByDate(const QString &familyId, const QString &date, const QString &mealType);
    QList<DailyMenu> getByFamilyDate(const QString &familyId, const QString &date);
    bool save(const DailyMenu &m);
    bool update(const DailyMenu &m);
    QSqlDatabase database() const { return db_; }

signals:
    void menuChanged(const QString &familyId, const QString &date);

private:
    static DailyMenu mapRow(const QSqlQuery &query);
    QSqlDatabase db_;
};

class MenuItemRepository : public QObject {
    Q_OBJECT
public:
    explicit MenuItemRepository(QObject *parent = nullptr);

    QList<MenuItem> getByMenu(const QString &menuId);
    bool addItem(const MenuItem &item);
    bool removeItem(const QString &menuId, const QString &recipeId);
    bool updateStatus(const QString &itemId, const QString &status);
    bool clearMenu(const QString &menuId);
    bool clearUnlocked(const QString &menuId);
    QSqlDatabase database() const { return db_; }

private:
    static MenuItem mapRow(const QSqlQuery &query);
    QSqlDatabase db_;
};

} // namespace smart_diet
