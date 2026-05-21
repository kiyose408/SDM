#include "tag_service.h"
#include "data/tag_repository.h"
#include "data/tag_entity.h"
#include "utils/uuid_utils.h"
#include "utils/time_utils.h"
#include <QSqlQuery>
#include <QDebug>

namespace smart_diet {

TagService::TagService(TagRepository *repo, QObject *parent)
    : BaseService(parent), repo_(repo) { seedTags(); }

QVariantList TagService::getAll() {
    QVariantList list;
    for (const auto &t : repo_->getAll()) {
        QVariantMap m;
        m[QStringLiteral("id")]   = t.id;
        m[QStringLiteral("name")] = t.name;
        m[QStringLiteral("type")] = t.type;
        list.append(m);
    }
    return list;
}

QVariantList TagService::getByRecipe(const QString &rid) {
    QVariantList list;
    for (const auto &t : repo_->getByRecipe(rid)) {
        QVariantMap m;
        m[QStringLiteral("id")]   = t.id;
        m[QStringLiteral("name")] = t.name;
        list.append(m);
    }
    return list;
}

QVariantList TagService::getTagIdsByRecipe(const QString &rid) {
    QVariantList list;
    for (const auto &id : repo_->getTagIdsByRecipe(rid)) list.append(id);
    return list;
}

bool TagService::setRecipeTags(const QString &rid, const QVariantList &tagIds) {
    QList<QString> ids;
    for (const auto &v : tagIds) ids.append(v.toString());
    return repo_->setRecipeTags(rid, ids);
}

void TagService::seedTags() {
    if (repo_->getAll().count() > 0) return;

    struct { const char *n, *t; } tags[] = {
        {"高蛋白", "nutrition"}, {"低脂", "nutrition"}, {"高纤维", "nutrition"},
        {"清淡", "flavor"}, {"重口", "flavor"}, {"微辣", "flavor"},
        {"快手菜", "cooking_method"}, {"炖煮", "cooking_method"}, {"煎炒", "cooking_method"},
        {"家常", "occasion"}, {"宴客", "occasion"}, {"减脂餐", "occasion"},
    };

    const QString now = utcNow();
    for (const auto &tag : tags) {
        Tag t;
        t.id         = generateUuid();
        t.name       = QString::fromUtf8(tag.n);
        t.type       = QString::fromUtf8(tag.t);
        t.created_at = now;
        // 直接 SQL 插入
        QSqlQuery q(repo_->database());
        q.prepare(QStringLiteral("INSERT INTO tags (id,name,type,created_at) VALUES (:id,:n,:t,:ca)"));
        q.bindValue(QStringLiteral(":id"), t.id);
        q.bindValue(QStringLiteral(":n"), t.name);
        q.bindValue(QStringLiteral(":t"), t.type);
        q.bindValue(QStringLiteral(":ca"), t.created_at);
        q.exec();
    }
    qDebug() << "[TagService] Seeded" << sizeof(tags)/sizeof(tags[0]) << "tags";
}

} // namespace smart_diet
