-- ============================================================================
-- SmartDietManager — SQLite 本地数据库建表脚本
-- 版本: v1.0  日期: 2026-05-20
-- 来源: 004.数据库设计文档 v1.9
-- 说明: 所有主键为 UUID (TEXT 36位)，时间戳为 ISO 8601 文本格式
-- ============================================================================

-- 启用外键约束（SQLite 默认关闭）
PRAGMA foreign_keys = ON;
PRAGMA journal_mode = WAL;

-- ============================================================================
-- 1. users — 用户表
-- ============================================================================
CREATE TABLE IF NOT EXISTS users (
    id              TEXT    PRIMARY KEY NOT NULL,   -- UUID
    login_id        TEXT    NOT NULL UNIQUE,        -- 登录标识（手机号/邮箱/用户名）
    nickname        TEXT    NOT NULL,               -- 用户昵称
    avatar_url      TEXT,                           -- 头像路径或 URL
    gender          INTEGER DEFAULT 0,              -- 0=未设置, 1=男, 2=女
    height          REAL,                           -- 身高 (cm)
    weight          REAL,                           -- 体重 (kg)
    birth_date      TEXT,                           -- 出生日期 YYYY-MM-DD
    diet_goal       TEXT    DEFAULT 'maintenance',  -- weight_loss / muscle_gain / maintenance
    bmr             REAL,                           -- 基础代谢率 (kcal/day)
    tdee            REAL,                           -- 每日总能量消耗 (kcal/day)
    password_hash   TEXT    NOT NULL,               -- Argon2id 哈希
    is_deleted      INTEGER DEFAULT 0,              -- 0=正常, 1=已删除
    version         INTEGER DEFAULT 1,              -- 乐观锁版本号
    created_at      TEXT    NOT NULL,               -- ISO 8601
    updated_at      TEXT    NOT NULL                -- ISO 8601
);
CREATE INDEX IF NOT EXISTS idx_users_login_id   ON users (login_id);
CREATE INDEX IF NOT EXISTS idx_users_nickname   ON users (nickname);
CREATE INDEX IF NOT EXISTS idx_users_is_deleted ON users (is_deleted);

-- ============================================================================
-- 2. families — 家庭表
-- ============================================================================
CREATE TABLE IF NOT EXISTS families (
    id                      TEXT    PRIMARY KEY NOT NULL,
    name                    TEXT    NOT NULL,               -- 默认 "{创建者昵称}的家"
    invite_code             TEXT    NOT NULL UNIQUE,        -- 6 位字母数字邀请码
    invite_code_expires_at  TEXT,                           -- NULL 表示永不过期
    creator_id              TEXT    NOT NULL,               -- FK → users.id
    is_deleted              INTEGER DEFAULT 0,
    version                 INTEGER DEFAULT 1,
    created_at              TEXT    NOT NULL,
    updated_at              TEXT    NOT NULL,
    FOREIGN KEY (creator_id) REFERENCES users (id)
);
CREATE INDEX IF NOT EXISTS idx_families_invite_code ON families (invite_code);
CREATE INDEX IF NOT EXISTS idx_families_creator     ON families (creator_id);

-- ============================================================================
-- 3. family_members — 家庭成员关系表
-- ============================================================================
CREATE TABLE IF NOT EXISTS family_members (
    id                  TEXT    PRIMARY KEY NOT NULL,
    family_id           TEXT    NOT NULL,       -- FK → families.id
    user_id             TEXT    NOT NULL,       -- FK → users.id
    role                TEXT    NOT NULL DEFAULT 'member',  -- owner / member
    can_manage_inventory INTEGER DEFAULT 0,     -- 冰箱入库权限（仅 member 生效）
    can_confirm_menu    INTEGER DEFAULT 0,     -- 菜单确认权（仅 member 生效）
    joined_at           TEXT    NOT NULL,
    version             INTEGER DEFAULT 1,
    updated_at          TEXT    NOT NULL,
    FOREIGN KEY (family_id) REFERENCES families (id),
    FOREIGN KEY (user_id)   REFERENCES users (id),
    UNIQUE (family_id, user_id)
);
CREATE INDEX IF NOT EXISTS idx_fm_family_id ON family_members (family_id);
CREATE INDEX IF NOT EXISTS idx_fm_user_id   ON family_members (user_id);
CREATE INDEX IF NOT EXISTS idx_fm_role      ON family_members (role);

-- ============================================================================
-- 4. ingredients — 食材表
-- ============================================================================
CREATE TABLE IF NOT EXISTS ingredients (
    id              TEXT    PRIMARY KEY NOT NULL,
    name            TEXT    NOT NULL,               -- 食材名称
    unit            TEXT    NOT NULL,               -- g 或 ml（仅此两种）
    calories_per_100 REAL,                          -- 每 100g/ml 热量 (kcal)
    protein_per_100  REAL,                          -- 每 100g/ml 蛋白质 (g)
    carbs_per_100    REAL,                          -- 每 100g/ml 碳水 (g)
    fat_per_100      REAL,                          -- 每 100g/ml 脂肪 (g)
    image_url       TEXT,                           -- 食材图片
    category        TEXT    NOT NULL,               -- vegetable/meat/seafood/staple/condiment/dairy/other
    is_system       INTEGER DEFAULT 1,              -- 0=用户自定义, 1=系统内置
    creator_id      TEXT,                           -- FK → users.id（自定义食材创建者）
    version         INTEGER DEFAULT 1,
    created_at      TEXT    NOT NULL,
    updated_at      TEXT    NOT NULL,
    FOREIGN KEY (creator_id) REFERENCES users (id)
);
CREATE INDEX IF NOT EXISTS idx_ingredients_name      ON ingredients (name);
CREATE INDEX IF NOT EXISTS idx_ingredients_category   ON ingredients (category);
CREATE INDEX IF NOT EXISTS idx_ingredients_is_system  ON ingredients (is_system);

-- ============================================================================
-- 5. recipes — 菜谱表
-- ============================================================================
CREATE TABLE IF NOT EXISTS recipes (
    id              TEXT    PRIMARY KEY NOT NULL,
    name            TEXT    NOT NULL,
    cover_image_url TEXT,
    description     TEXT,
    cooking_time    INTEGER,                        -- 烹饪时长 (分钟)
    servings        INTEGER DEFAULT 2,              -- 默认几人份
    meal_type       TEXT    DEFAULT 'any',          -- breakfast / lunch / dinner / any
    total_calories  REAL,                           -- 反范式化缓存：总热量 (kcal)
    total_protein   REAL,                           -- 反范式化缓存：总蛋白质 (g)
    total_carbs     REAL,                           -- 反范式化缓存：总碳水 (g)
    total_fat       REAL,                           -- 反范式化缓存：总脂肪 (g)
    is_system       INTEGER DEFAULT 1,              -- 0=用户自定义, 1=系统内置
    creator_id      TEXT,                           -- FK → users.id
    family_id       TEXT,                           -- FK → families.id（家庭共享菜谱）
    popularity      INTEGER DEFAULT 0,              -- 累计被选次数
    is_deleted      INTEGER DEFAULT 0,
    version         INTEGER DEFAULT 1,
    created_at      TEXT    NOT NULL,
    updated_at      TEXT    NOT NULL,
    FOREIGN KEY (creator_id) REFERENCES users (id),
    FOREIGN KEY (family_id)  REFERENCES families (id)
);
CREATE INDEX IF NOT EXISTS idx_recipes_name        ON recipes (name);
CREATE INDEX IF NOT EXISTS idx_recipes_meal_type   ON recipes (meal_type);
CREATE INDEX IF NOT EXISTS idx_recipes_is_system   ON recipes (is_system);
CREATE INDEX IF NOT EXISTS idx_recipes_family_id   ON recipes (family_id);
CREATE INDEX IF NOT EXISTS idx_recipes_popularity  ON recipes (popularity DESC);

-- ============================================================================
-- 6. recipe_ingredients — 菜谱食材关联表
-- ============================================================================
CREATE TABLE IF NOT EXISTS recipe_ingredients (
    id              TEXT    PRIMARY KEY NOT NULL,
    recipe_id       TEXT    NOT NULL,       -- FK → recipes.id
    ingredient_id   TEXT    NOT NULL,       -- FK → ingredients.id
    amount          REAL    NOT NULL,       -- 所需用量（g 或 ml，单位继承 ingredients.unit）
    created_at      TEXT    NOT NULL,
    FOREIGN KEY (recipe_id)     REFERENCES recipes (id),
    FOREIGN KEY (ingredient_id) REFERENCES ingredients (id),
    UNIQUE (recipe_id, ingredient_id)
);
CREATE INDEX IF NOT EXISTS idx_ri_recipe_id     ON recipe_ingredients (recipe_id);
CREATE INDEX IF NOT EXISTS idx_ri_ingredient_id ON recipe_ingredients (ingredient_id);

-- ============================================================================
-- 7. recipe_steps — 菜谱步骤表
-- ============================================================================
CREATE TABLE IF NOT EXISTS recipe_steps (
    id              TEXT    PRIMARY KEY NOT NULL,
    recipe_id       TEXT    NOT NULL,       -- FK → recipes.id
    step_number     INTEGER NOT NULL,       -- 步骤序号（从 1 开始）
    description     TEXT    NOT NULL,       -- 步骤文字说明
    image_url       TEXT,                   -- 步骤配图
    created_at      TEXT    NOT NULL,
    FOREIGN KEY (recipe_id) REFERENCES recipes (id),
    UNIQUE (recipe_id, step_number)
);
CREATE INDEX IF NOT EXISTS idx_rs_recipe_id ON recipe_steps (recipe_id);

-- ============================================================================
-- 8. tags — 标签表
-- ============================================================================
CREATE TABLE IF NOT EXISTS tags (
    id          TEXT    PRIMARY KEY NOT NULL,
    name        TEXT    NOT NULL UNIQUE,    -- 标签名（咸鲜、微辣、快手菜...）
    type        TEXT    NOT NULL,           -- flavor / cooking_method / occasion
    created_at  TEXT    NOT NULL
);
CREATE INDEX IF NOT EXISTS idx_tags_type ON tags (type);

-- ============================================================================
-- 9. recipe_tag_relations — 菜谱标签关联表
-- ============================================================================
CREATE TABLE IF NOT EXISTS recipe_tag_relations (
    id          TEXT    PRIMARY KEY NOT NULL,
    recipe_id   TEXT    NOT NULL,       -- FK → recipes.id
    tag_id      TEXT    NOT NULL,       -- FK → tags.id
    created_at  TEXT    NOT NULL,
    FOREIGN KEY (recipe_id) REFERENCES recipes (id),
    FOREIGN KEY (tag_id)    REFERENCES tags (id),
    UNIQUE (recipe_id, tag_id)
);
CREATE INDEX IF NOT EXISTS idx_rtr_recipe_id ON recipe_tag_relations (recipe_id);
CREATE INDEX IF NOT EXISTS idx_rtr_tag_id    ON recipe_tag_relations (tag_id);

-- ============================================================================
-- 10. recipe_favorites — 菜谱收藏表
-- ============================================================================
CREATE TABLE IF NOT EXISTS recipe_favorites (
    id          TEXT    PRIMARY KEY NOT NULL,
    user_id     TEXT    NOT NULL,       -- FK → users.id
    recipe_id   TEXT    NOT NULL,       -- FK → recipes.id
    created_at  TEXT    NOT NULL,
    FOREIGN KEY (user_id)   REFERENCES users (id),
    FOREIGN KEY (recipe_id) REFERENCES recipes (id),
    UNIQUE (user_id, recipe_id)
);
CREATE INDEX IF NOT EXISTS idx_fav_user_id   ON recipe_favorites (user_id);
CREATE INDEX IF NOT EXISTS idx_fav_recipe_id ON recipe_favorites (recipe_id);

-- ============================================================================
-- 11. daily_menus — 每日菜单表
-- ============================================================================
CREATE TABLE IF NOT EXISTS daily_menus (
    id              TEXT    PRIMARY KEY NOT NULL,
    family_id       TEXT    NOT NULL,               -- FK → families.id
    date            TEXT    NOT NULL,               -- YYYY-MM-DD
    meal_type       TEXT    NOT NULL,               -- breakfast / lunch / dinner
    diner_count     INTEGER DEFAULT 1,              -- 用餐人数
    diet_mode       TEXT    DEFAULT 'maintenance',  -- weight_loss/muscle_gain/maintenance/gathering/cheat_day
    is_confirmed    INTEGER DEFAULT 0,              -- 掌勺人是否确认
    confirmed_by    TEXT,                           -- FK → users.id（确认人）
    confirmed_at    TEXT,                           -- 确认时间
    chef_id         TEXT,                           -- FK → users.id（当前掌勺人）
    is_deleted      INTEGER DEFAULT 0,
    version         INTEGER DEFAULT 1,
    created_at      TEXT    NOT NULL,
    updated_at      TEXT    NOT NULL,
    FOREIGN KEY (family_id)    REFERENCES families (id),
    FOREIGN KEY (confirmed_by) REFERENCES users (id),
    FOREIGN KEY (chef_id)      REFERENCES users (id),
    UNIQUE (family_id, date, meal_type)
);
CREATE INDEX IF NOT EXISTS idx_dm_family_date ON daily_menus (family_id, date);
CREATE INDEX IF NOT EXISTS idx_dm_chef_id     ON daily_menus (chef_id);

-- ============================================================================
-- 12. menu_items — 菜单菜品关联表
-- ============================================================================
CREATE TABLE IF NOT EXISTS menu_items (
    id                  TEXT    PRIMARY KEY NOT NULL,
    menu_id             TEXT    NOT NULL,       -- FK → daily_menus.id
    recipe_id           TEXT    NOT NULL,       -- FK → recipes.id
    is_locked           INTEGER DEFAULT 0,      -- 锁定后不参与"换一换"刷新
    servings_override   INTEGER,               -- 覆盖菜谱默认份数，NULL 则使用 recipe.servings
    status              TEXT    DEFAULT 'pending', -- pending / cooking / completed
    sort_order          INTEGER DEFAULT 0,      -- 展示排序
    version             INTEGER DEFAULT 1,
    created_at          TEXT    NOT NULL,
    updated_at          TEXT    NOT NULL,
    FOREIGN KEY (menu_id)   REFERENCES daily_menus (id),
    FOREIGN KEY (recipe_id) REFERENCES recipes (id),
    UNIQUE (menu_id, recipe_id)
);
CREATE INDEX IF NOT EXISTS idx_mi_menu_id ON menu_items (menu_id);
CREATE INDEX IF NOT EXISTS idx_mi_status   ON menu_items (status);

-- ============================================================================
-- 13. wishlist_items — 心愿单表
-- ============================================================================
CREATE TABLE IF NOT EXISTS wishlist_items (
    id              TEXT    PRIMARY KEY NOT NULL,
    family_id       TEXT    NOT NULL,       -- FK → families.id
    recipe_id       TEXT    NOT NULL,       -- FK → recipes.id
    suggested_by    TEXT    NOT NULL,       -- FK → users.id（推荐人）
    meal_type       TEXT,                   -- 建议餐别（可选）
    is_adopted      INTEGER DEFAULT 0,      -- 是否已被掌勺人采纳
    adopted_at      TEXT,                   -- 采纳时间
    adopted_menu_id TEXT,                   -- FK → daily_menus.id
    created_at      TEXT    NOT NULL,
    FOREIGN KEY (family_id)        REFERENCES families (id),
    FOREIGN KEY (recipe_id)        REFERENCES recipes (id),
    FOREIGN KEY (suggested_by)     REFERENCES users (id),
    FOREIGN KEY (adopted_menu_id)  REFERENCES daily_menus (id),
    UNIQUE (family_id, recipe_id, suggested_by)
);
CREATE INDEX IF NOT EXISTS idx_wl_family_id     ON wishlist_items (family_id);
CREATE INDEX IF NOT EXISTS idx_wl_is_adopted    ON wishlist_items (is_adopted);
CREATE INDEX IF NOT EXISTS idx_wl_suggested_by  ON wishlist_items (suggested_by);

-- ============================================================================
-- 14. inventory_batches — 库存批次表
-- ============================================================================
CREATE TABLE IF NOT EXISTS inventory_batches (
    id              TEXT    PRIMARY KEY NOT NULL,
    family_id       TEXT    NOT NULL,               -- FK → families.id
    ingredient_id   TEXT    NOT NULL,               -- FK → ingredients.id
    batch_quantity  REAL    NOT NULL,               -- 本批次当前数量
    unit            TEXT    NOT NULL,               -- g 或 ml（与 ingredients.unit 一致）
    expiry_date     TEXT,                           -- 本批次过期日期
    purchase_date   TEXT,                           -- 采购/入库日期
    source          TEXT    DEFAULT 'manual_add',   -- 入库来源
    added_by        TEXT,                           -- FK → users.id（入库操作人）
    is_deleted      INTEGER DEFAULT 0,
    version         INTEGER DEFAULT 1,
    created_at      TEXT    NOT NULL,
    updated_at      TEXT    NOT NULL,
    FOREIGN KEY (family_id)     REFERENCES families (id),
    FOREIGN KEY (ingredient_id) REFERENCES ingredients (id),
    FOREIGN KEY (added_by)      REFERENCES users (id),
    UNIQUE (family_id, ingredient_id, purchase_date, expiry_date)
);
CREATE INDEX IF NOT EXISTS idx_ib_family_id          ON inventory_batches (family_id);
CREATE INDEX IF NOT EXISTS idx_ib_ingredient_id      ON inventory_batches (ingredient_id);
CREATE INDEX IF NOT EXISTS idx_ib_expiry_date        ON inventory_batches (expiry_date);
CREATE INDEX IF NOT EXISTS idx_ib_family_ingredient   ON inventory_batches (family_id, ingredient_id);
CREATE INDEX IF NOT EXISTS idx_ib_fifo               ON inventory_batches (family_id, ingredient_id, purchase_date ASC, expiry_date ASC);

-- ============================================================================
-- 15. inventory_logs — 库存操作日志表
-- ============================================================================
CREATE TABLE IF NOT EXISTS inventory_logs (
    id                      TEXT    PRIMARY KEY NOT NULL,
    family_id               TEXT    NOT NULL,       -- FK → families.id
    batch_id                TEXT    NOT NULL,       -- FK → inventory_batches.id
    ingredient_id           TEXT    NOT NULL,       -- FK → ingredients.id（冗余，便于查询）
    operation               TEXT    NOT NULL,       -- add / deduct / edit / clear
    quantity_change         REAL    NOT NULL,       -- 正=入库, 负=核销
    quantity_before         REAL    NOT NULL,       -- 操作前批次库存
    quantity_after          REAL    NOT NULL,       -- 操作后批次库存
    reason                  TEXT    NOT NULL,       -- manual_add/purchase_list_batch/scan_add/cooking_deduction/manual_edit/manual_clear
    related_menu_id         TEXT,                   -- FK → daily_menus.id
    operator_id             TEXT    NOT NULL,       -- FK → users.id
    snapshot_recipe_amount  REAL,                   -- 核销时的食材用量快照
    snapshot_servings       REAL,                   -- 核销时的份数快照
    snapshot_recipe_name    TEXT,                   -- 核销时的菜谱名称快照
    snapshot_ingredient_name TEXT,                  -- 核销时的食材名称快照
    created_at              TEXT    NOT NULL,
    updated_at              TEXT    NOT NULL,
    FOREIGN KEY (family_id)      REFERENCES families (id),
    FOREIGN KEY (batch_id)       REFERENCES inventory_batches (id),
    FOREIGN KEY (ingredient_id)  REFERENCES ingredients (id),
    FOREIGN KEY (related_menu_id) REFERENCES daily_menus (id),
    FOREIGN KEY (operator_id)    REFERENCES users (id)
);
CREATE INDEX IF NOT EXISTS idx_il_batch_id        ON inventory_logs (batch_id);
CREATE INDEX IF NOT EXISTS idx_il_family_id       ON inventory_logs (family_id);
CREATE INDEX IF NOT EXISTS idx_il_created_at      ON inventory_logs (created_at);
CREATE INDEX IF NOT EXISTS idx_il_related_menu    ON inventory_logs (related_menu_id);
CREATE INDEX IF NOT EXISTS idx_il_family_created  ON inventory_logs (family_id, created_at);

-- ============================================================================
-- 16. shopping_list_items — 采购清单项表
-- ============================================================================
CREATE TABLE IF NOT EXISTS shopping_list_items (
    id                  TEXT    PRIMARY KEY NOT NULL,
    family_id           TEXT    NOT NULL,       -- FK → families.id
    ingredient_id       TEXT    NOT NULL,       -- FK → ingredients.id
    required_quantity   REAL    NOT NULL,       -- 需采购量（g 或 ml）
    is_purchased        INTEGER DEFAULT 0,      -- 是否已购买
    purchased_at        TEXT,                   -- 购买时间
    source_menu_id      TEXT,                   -- FK → daily_menus.id
    created_at          TEXT    NOT NULL,
    updated_at          TEXT    NOT NULL,
    FOREIGN KEY (family_id)      REFERENCES families (id),
    FOREIGN KEY (ingredient_id)  REFERENCES ingredients (id),
    FOREIGN KEY (source_menu_id) REFERENCES daily_menus (id)
);
CREATE INDEX IF NOT EXISTS idx_sli_family_id     ON shopping_list_items (family_id);
CREATE INDEX IF NOT EXISTS idx_sli_is_purchased  ON shopping_list_items (is_purchased);

-- ============================================================================
-- 17. consumption_records — 饮食消费记录表
-- ============================================================================
CREATE TABLE IF NOT EXISTS consumption_records (
    id              TEXT    PRIMARY KEY NOT NULL,
    user_id         TEXT    NOT NULL,           -- FK → users.id
    family_id       TEXT    NOT NULL,           -- FK → families.id
    date            TEXT    NOT NULL,           -- YYYY-MM-DD
    meal_type       TEXT    NOT NULL,           -- breakfast / lunch / dinner / snack
    recipe_id       TEXT,                       -- FK → recipes.id
    servings        REAL    DEFAULT 1.0,        -- 食用份数
    calories        REAL,                       -- 摄入热量 (kcal)
    protein         REAL,                       -- 摄入蛋白质 (g)
    carbs           REAL,                       -- 摄入碳水 (g)
    fat             REAL,                       -- 摄入脂肪 (g)
    menu_item_id    TEXT,                       -- FK → menu_items.id（核销产生）
    is_deleted      INTEGER DEFAULT 0,          -- 支持撤销记录
    version         INTEGER DEFAULT 1,
    created_at      TEXT    NOT NULL,
    updated_at      TEXT    NOT NULL,
    FOREIGN KEY (user_id)      REFERENCES users (id),
    FOREIGN KEY (family_id)    REFERENCES families (id),
    FOREIGN KEY (recipe_id)    REFERENCES recipes (id),
    FOREIGN KEY (menu_item_id) REFERENCES menu_items (id)
);
CREATE INDEX IF NOT EXISTS idx_cr_user_date   ON consumption_records (user_id, date);
CREATE INDEX IF NOT EXISTS idx_cr_family_date ON consumption_records (family_id, date);
CREATE INDEX IF NOT EXISTS idx_cr_meal_type   ON consumption_records (meal_type);

-- ============================================================================
-- 18. taste_preferences — 口味偏好表
-- ============================================================================
CREATE TABLE IF NOT EXISTS taste_preferences (
    id          TEXT    PRIMARY KEY NOT NULL,
    user_id     TEXT    NOT NULL UNIQUE,    -- FK → users.id
    salty       REAL    DEFAULT 0.5,        -- 咸 (0.0–1.0)
    sweet       REAL    DEFAULT 0.5,        -- 甜 (0.0–1.0)
    sour        REAL    DEFAULT 0.5,        -- 酸 (0.0–1.0)
    spicy       REAL    DEFAULT 0.5,        -- 辣 (0.0–1.0)
    umami       REAL    DEFAULT 0.5,        -- 鲜 (0.0–1.0)
    version     INTEGER DEFAULT 1,
    updated_at  TEXT    NOT NULL,
    FOREIGN KEY (user_id) REFERENCES users (id)
);
CREATE INDEX IF NOT EXISTS idx_tp_user_id ON taste_preferences (user_id);

-- ============================================================================
-- 19. ingredient_blacklist — 食材黑名单表
-- ============================================================================
CREATE TABLE IF NOT EXISTS ingredient_blacklist (
    id              TEXT    PRIMARY KEY NOT NULL,
    user_id         TEXT    NOT NULL,       -- FK → users.id
    ingredient_id   TEXT    NOT NULL,       -- FK → ingredients.id
    created_at      TEXT    NOT NULL,
    FOREIGN KEY (user_id)       REFERENCES users (id),
    FOREIGN KEY (ingredient_id) REFERENCES ingredients (id),
    UNIQUE (user_id, ingredient_id)
);
CREATE INDEX IF NOT EXISTS idx_ibl_user_id ON ingredient_blacklist (user_id);

-- ============================================================================
-- 20. user_allergens — 用户过敏源表
-- ============================================================================
CREATE TABLE IF NOT EXISTS user_allergens (
    id              TEXT    PRIMARY KEY NOT NULL,
    user_id         TEXT    NOT NULL,               -- FK → users.id
    ingredient_id   TEXT    NOT NULL,               -- FK → ingredients.id
    severity        TEXT    DEFAULT 'mild',         -- mild / moderate / severe
    created_at      TEXT    NOT NULL,
    FOREIGN KEY (user_id)       REFERENCES users (id),
    FOREIGN KEY (ingredient_id) REFERENCES ingredients (id),
    UNIQUE (user_id, ingredient_id)
);
CREATE INDEX IF NOT EXISTS idx_ua_user_id ON user_allergens (user_id);

-- ============================================================================
-- 21. nutrition_profiles — 营养档案表
-- ============================================================================
CREATE TABLE IF NOT EXISTS nutrition_profiles (
    id                      TEXT    PRIMARY KEY NOT NULL,
    user_id                 TEXT    NOT NULL UNIQUE,    -- FK → users.id
    daily_calorie_target    REAL,                       -- 自定义每日热量目标，NULL 则使用 users.tdee
    calorie_deficit         REAL    DEFAULT 0,          -- 自定义热量缺口 (kcal)
    protein_ratio           REAL    DEFAULT 0.30,       -- 蛋白质供能比
    carbs_ratio             REAL    DEFAULT 0.45,       -- 碳水供能比
    fat_ratio               REAL    DEFAULT 0.25,       -- 脂肪供能比
    reminder_interval       INTEGER DEFAULT 14,         -- 身体数据更新提醒周期 (天)
    next_reminder_at        TEXT,                       -- 下次提醒时间
    version                 INTEGER DEFAULT 1,
    updated_at              TEXT    NOT NULL,
    FOREIGN KEY (user_id) REFERENCES users (id)
);

-- ============================================================================
-- 22. health_reports — 统计报告表
-- ============================================================================
CREATE TABLE IF NOT EXISTS health_reports (
    id              TEXT    PRIMARY KEY NOT NULL,
    family_id       TEXT    NOT NULL,       -- FK → families.id
    user_id         TEXT,                   -- FK → users.id（个人报告时指定，家庭报告时为 NULL）
    report_type     TEXT    NOT NULL,       -- weekly / monthly
    report_scope    TEXT    NOT NULL,       -- personal / family
    start_date      TEXT    NOT NULL,       -- YYYY-MM-DD
    end_date        TEXT    NOT NULL,       -- YYYY-MM-DD
    report_data     TEXT    NOT NULL,       -- JSON（SQLite 为 TEXT）
    generated_at    TEXT    NOT NULL,
    FOREIGN KEY (family_id) REFERENCES families (id),
    FOREIGN KEY (user_id)   REFERENCES users (id)
);
CREATE INDEX IF NOT EXISTS idx_hr_family_id ON health_reports (family_id);
CREATE INDEX IF NOT EXISTS idx_hr_user_id   ON health_reports (user_id);
CREATE INDEX IF NOT EXISTS idx_hr_period    ON health_reports (start_date, end_date);

-- ============================================================================
-- 23. sync_queue — 同步队列表（仅本地 SQLite）
-- ============================================================================
CREATE TABLE IF NOT EXISTS sync_queue (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    entity_type     TEXT    NOT NULL,               -- 实体表名
    entity_id       TEXT    NOT NULL,               -- 实体 UUID
    operation       TEXT    NOT NULL,               -- INSERT / UPDATE / DELETE
    payload         TEXT    NOT NULL,               -- 变更数据的完整 JSON
    base_version    INTEGER NOT NULL,               -- 同步时基于的实体版本号（乐观锁）
    retry_count     INTEGER DEFAULT 0,              -- 重试次数
    max_retries     INTEGER DEFAULT 5,              -- 最大重试次数
    status          TEXT    DEFAULT 'pending',      -- pending / syncing / completed / failed
    error_message   TEXT,                           -- 最近一次错误信息
    created_at      TEXT    NOT NULL,
    last_retry_at   TEXT
);
CREATE INDEX IF NOT EXISTS idx_sq_status           ON sync_queue (status);
CREATE INDEX IF NOT EXISTS idx_sq_entity_type_id   ON sync_queue (entity_type, entity_id);
CREATE INDEX IF NOT EXISTS idx_sq_created_at       ON sync_queue (created_at);
