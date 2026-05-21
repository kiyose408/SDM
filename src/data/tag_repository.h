#pragma once
#include <QObject>
#include <QSqlDatabase>
#include <QList>
#include <QString>

namespace smart_diet {

class TagRepository : public QObject {
    Q_OBJECT
public:
    explicit TagRepository(QObject *parent = nullptr);

    QList<struct Tag> getAll();
    QList<struct Tag> getByRecipe(const QString &recipeId);
    bool setRecipeTags(const QString &recipeId, const QList<QString> &tagIds);
    QList<QString> getTagIdsByRecipe(const QString &recipeId);
    QSqlDatabase database() const { return db_; }

private:
    static Tag mapRow(const QSqlQuery &query);
    QSqlDatabase db_;
};

} // namespace smart_diet
