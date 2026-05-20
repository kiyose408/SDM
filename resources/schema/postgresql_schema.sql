-- ============================================================================
-- SmartDietManager — PostgreSQL 云端数据库建表脚本
-- 版本: v1.0  日期: 2026-05-20
-- 来源: 004.数据库设计文档 v1.9
-- 说明: 与 SQLite schema 表结构一致，差异仅在于类型适配和 JSONB 支持
--       本脚本在阶段 8 部署到云服务器，阶段 1-7 仅维护作为参考
-- ============================================================================

-- ============================================================================
-- 1. users
-- ============================================================================
CREATE TABLE IF NOT EXISTS users (
    id              TEXT        PRIMARY KEY,
    login_id        TEXT        NOT NULL UNIQUE,
    nickname        TEXT        NOT NULL,
    avatar_url      TEXT,
    gender          SMALLINT    DEFAULT 0,
    height          REAL,
    weight          REAL,
    birth_date      TEXT,
    diet_goal       TEXT        DEFAULT 'maintenance',
    bmr             REAL,
    tdee            REAL,
    password_hash   TEXT        NOT NULL,
    is_deleted      BOOLEAN     DEFAULT FALSE,
    version         INTEGER     DEFAULT 1,
    created_at      TIMESTAMPTZ NOT NULL,
    updated_at      TIMESTAMPTZ NOT NULL
);
CREATE INDEX IF NOT EXISTS idx_users_login_id   ON users (login_id);
CREATE INDEX IF NOT EXISTS idx_users_is_deleted ON users (is_deleted);

-- ============================================================================
-- 2. families
-- ============================================================================
CREATE TABLE IF NOT EXISTS families (
    id                      TEXT        PRIMARY KEY,
    name                    TEXT        NOT NULL,
    invite_code             TEXT        NOT NULL UNIQUE,
    invite_code_expires_at  TIMESTAMPTZ,
    creator_id              TEXT        NOT NULL REFERENCES users (id),
    is_deleted              BOOLEAN     DEFAULT FALSE,
    version                 INTEGER     DEFAULT 1,
    created_at              TIMESTAMPTZ NOT NULL,
    updated_at              TIMESTAMPTZ NOT NULL
);
CREATE INDEX IF NOT EXISTS idx_families_invite_code ON families (invite_code);
CREATE INDEX IF NOT EXISTS idx_families_creator     ON families (creator_id);

-- ============================================================================
-- 3. family_members
-- ============================================================================
CREATE TABLE IF NOT EXISTS family_members (
    id                  TEXT        PRIMARY KEY,
    family_id           TEXT        NOT NULL REFERENCES families (id),
    user_id             TEXT        NOT NULL REFERENCES users (id),
    role                TEXT        NOT NULL DEFAULT 'member',
    can_manage_inventory BOOLEAN     DEFAULT FALSE,
    can_confirm_menu    BOOLEAN     DEFAULT FALSE,
    joined_at           TIMESTAMPTZ NOT NULL,
    version             INTEGER     DEFAULT 1,
    updated_at          TIMESTAMPTZ NOT NULL,
    UNIQUE (family_id, user_id)
);
CREATE INDEX IF NOT EXISTS idx_fm_family_id ON family_members (family_id);
CREATE INDEX IF NOT EXISTS idx_fm_user_id   ON family_members (user_id);
CREATE INDEX IF NOT EXISTS idx_fm_role      ON family_members (role);

-- ============================================================================
-- 4. ingredients
-- ============================================================================
CREATE TABLE IF NOT EXISTS ingredients (
    id              TEXT        PRIMARY KEY,
    name            TEXT        NOT NULL,
    unit            TEXT        NOT NULL,
    calories_per_100 REAL,
    protein_per_100  REAL,
    carbs_per_100    REAL,
    fat_per_100      REAL,
    image_url       TEXT,
    category        TEXT        NOT NULL,
    is_system       BOOLEAN     DEFAULT TRUE,
    creator_id      TEXT        REFERENCES users (id),
    version         INTEGER     DEFAULT 1,
    created_at      TIMESTAMPTZ NOT NULL,
    updated_at      TIMESTAMPTZ NOT NULL
);
CREATE INDEX IF NOT EXISTS idx_ingredients_category  ON ingredients (category);
CREATE INDEX IF NOT EXISTS idx_ingredients_is_system ON ingredients (is_system);

-- ============================================================================
-- 5. recipes
-- ============================================================================
CREATE TABLE IF NOT EXISTS recipes (
    id              TEXT        PRIMARY KEY,
    name            TEXT        NOT NULL,
    cover_image_url TEXT,
    description     TEXT,
    cooking_time    INTEGER,
    servings        INTEGER     DEFAULT 2,
    meal_type       TEXT        DEFAULT 'any',
    total_calories  REAL,
    total_protein   REAL,
    total_carbs     REAL,
    total_fat       REAL,
    is_system       BOOLEAN     DEFAULT TRUE,
    creator_id      TEXT        REFERENCES users (id),
    family_id       TEXT        REFERENCES families (id),
    popularity      INTEGER     DEFAULT 0,
    is_deleted      BOOLEAN     DEFAULT FALSE,
    version         INTEGER     DEFAULT 1,
    created_at      TIMESTAMPTZ NOT NULL,
    updated_at      TIMESTAMPTZ NOT NULL
);
CREATE INDEX IF NOT EXISTS idx_recipes_meal_type  ON recipes (meal_type);
CREATE INDEX IF NOT EXISTS idx_recipes_is_system  ON recipes (is_system);
CREATE INDEX IF NOT EXISTS idx_recipes_family_id  ON recipes (family_id);
CREATE INDEX IF NOT EXISTS idx_recipes_popularity ON recipes (popularity DESC);

-- ============================================================================
-- 6. recipe_ingredients
-- ============================================================================
CREATE TABLE IF NOT EXISTS recipe_ingredients (
    id              TEXT        PRIMARY KEY,
    recipe_id       TEXT        NOT NULL REFERENCES recipes (id),
    ingredient_id   TEXT        NOT NULL REFERENCES ingredients (id),
    amount          REAL        NOT NULL,
    created_at      TIMESTAMPTZ NOT NULL,
    UNIQUE (recipe_id, ingredient_id)
);
CREATE INDEX IF NOT EXISTS idx_ri_recipe_id     ON recipe_ingredients (recipe_id);
CREATE INDEX IF NOT EXISTS idx_ri_ingredient_id ON recipe_ingredients (ingredient_id);

-- ============================================================================
-- 7. recipe_steps
-- ============================================================================
CREATE TABLE IF NOT EXISTS recipe_steps (
    id              TEXT        PRIMARY KEY,
    recipe_id       TEXT        NOT NULL REFERENCES recipes (id),
    step_number     INTEGER     NOT NULL,
    description     TEXT        NOT NULL,
    image_url       TEXT,
    created_at      TIMESTAMPTZ NOT NULL,
    UNIQUE (recipe_id, step_number)
);
CREATE INDEX IF NOT EXISTS idx_rs_recipe_id ON recipe_steps (recipe_id);

-- ============================================================================
-- 8. tags
-- ============================================================================
CREATE TABLE IF NOT EXISTS tags (
    id          TEXT        PRIMARY KEY,
    name        TEXT        NOT NULL UNIQUE,
    type        TEXT        NOT NULL,
    created_at  TIMESTAMPTZ NOT NULL
);
CREATE INDEX IF NOT EXISTS idx_tags_type ON tags (type);

-- ============================================================================
-- 9. recipe_tag_relations
-- ============================================================================
CREATE TABLE IF NOT EXISTS recipe_tag_relations (
    id          TEXT        PRIMARY KEY,
    recipe_id   TEXT        NOT NULL REFERENCES recipes (id),
    tag_id      TEXT        NOT NULL REFERENCES tags (id),
    created_at  TIMESTAMPTZ NOT NULL,
    UNIQUE (recipe_id, tag_id)
);
CREATE INDEX IF NOT EXISTS idx_rtr_recipe_id ON recipe_tag_relations (recipe_id);
CREATE INDEX IF NOT EXISTS idx_rtr_tag_id    ON recipe_tag_relations (tag_id);

-- ============================================================================
-- 10. recipe_favorites
-- ============================================================================
CREATE TABLE IF NOT EXISTS recipe_favorites (
    id          TEXT        PRIMARY KEY,
    user_id     TEXT        NOT NULL REFERENCES users (id),
    recipe_id   TEXT        NOT NULL REFERENCES recipes (id),
    created_at  TIMESTAMPTZ NOT NULL,
    UNIQUE (user_id, recipe_id)
);
CREATE INDEX IF NOT EXISTS idx_fav_user_id   ON recipe_favorites (user_id);
CREATE INDEX IF NOT EXISTS idx_fav_recipe_id ON recipe_favorites (recipe_id);

-- ============================================================================
-- 11. daily_menus
-- ============================================================================
CREATE TABLE IF NOT EXISTS daily_menus (
    id              TEXT        PRIMARY KEY,
    family_id       TEXT        NOT NULL REFERENCES families (id),
    date            DATE        NOT NULL,
    meal_type       TEXT        NOT NULL,
    diner_count     INTEGER     DEFAULT 1,
    diet_mode       TEXT        DEFAULT 'maintenance',
    is_confirmed    BOOLEAN     DEFAULT FALSE,
    confirmed_by    TEXT        REFERENCES users (id),
    confirmed_at    TIMESTAMPTZ,
    chef_id         TEXT        REFERENCES users (id),
    is_deleted      BOOLEAN     DEFAULT FALSE,
    version         INTEGER     DEFAULT 1,
    created_at      TIMESTAMPTZ NOT NULL,
    updated_at      TIMESTAMPTZ NOT NULL,
    UNIQUE (family_id, date, meal_type)
);
CREATE INDEX IF NOT EXISTS idx_dm_family_date ON daily_menus (family_id, date);
CREATE INDEX IF NOT EXISTS idx_dm_chef_id     ON daily_menus (chef_id);

-- ============================================================================
-- 12. menu_items
-- ============================================================================
CREATE TABLE IF NOT EXISTS menu_items (
    id                  TEXT        PRIMARY KEY,
    menu_id             TEXT        NOT NULL REFERENCES daily_menus (id),
    recipe_id           TEXT        NOT NULL REFERENCES recipes (id),
    is_locked           BOOLEAN     DEFAULT FALSE,
    servings_override   INTEGER,
    status              TEXT        DEFAULT 'pending',
    sort_order          INTEGER     DEFAULT 0,
    version             INTEGER     DEFAULT 1,
    created_at          TIMESTAMPTZ NOT NULL,
    updated_at          TIMESTAMPTZ NOT NULL,
    UNIQUE (menu_id, recipe_id)
);
CREATE INDEX IF NOT EXISTS idx_mi_menu_id ON menu_items (menu_id);
CREATE INDEX IF NOT EXISTS idx_mi_status   ON menu_items (status);

-- ============================================================================
-- 13. wishlist_items
-- ============================================================================
CREATE TABLE IF NOT EXISTS wishlist_items (
    id              TEXT        PRIMARY KEY,
    family_id       TEXT        NOT NULL REFERENCES families (id),
    recipe_id       TEXT        NOT NULL REFERENCES recipes (id),
    suggested_by    TEXT        NOT NULL REFERENCES users (id),
    meal_type       TEXT,
    is_adopted      BOOLEAN     DEFAULT FALSE,
    adopted_at      TIMESTAMPTZ,
    adopted_menu_id TEXT        REFERENCES daily_menus (id),
    created_at      TIMESTAMPTZ NOT NULL,
    UNIQUE (family_id, recipe_id, suggested_by)
);
CREATE INDEX IF NOT EXISTS idx_wl_family_id    ON wishlist_items (family_id);
CREATE INDEX IF NOT EXISTS idx_wl_is_adopted   ON wishlist_items (is_adopted);
CREATE INDEX IF NOT EXISTS idx_wl_suggested_by ON wishlist_items (suggested_by);

-- ============================================================================
-- 14. inventory_batches
-- ============================================================================
CREATE TABLE IF NOT EXISTS inventory_batches (
    id              TEXT        PRIMARY KEY,
    family_id       TEXT        NOT NULL REFERENCES families (id),
    ingredient_id   TEXT        NOT NULL REFERENCES ingredients (id),
    batch_quantity  REAL        NOT NULL,
    unit            TEXT        NOT NULL,
    expiry_date     DATE,
    purchase_date   DATE,
    source          TEXT        DEFAULT 'manual_add',
    added_by        TEXT        REFERENCES users (id),
    is_deleted      BOOLEAN     DEFAULT FALSE,
    version         INTEGER     DEFAULT 1,
    created_at      TIMESTAMPTZ NOT NULL,
    updated_at      TIMESTAMPTZ NOT NULL,
    UNIQUE (family_id, ingredient_id, purchase_date, expiry_date)
);
CREATE INDEX IF NOT EXISTS idx_ib_family_ingredient ON inventory_batches (family_id, ingredient_id);
CREATE INDEX IF NOT EXISTS idx_ib_expiry_date       ON inventory_batches (expiry_date);
CREATE INDEX IF NOT EXISTS idx_ib_fifo              ON inventory_batches (family_id, ingredient_id, purchase_date ASC, expiry_date ASC);

-- ============================================================================
-- 15. inventory_logs
-- ============================================================================
CREATE TABLE IF NOT EXISTS inventory_logs (
    id                      TEXT        PRIMARY KEY,
    family_id               TEXT        NOT NULL REFERENCES families (id),
    batch_id                TEXT        NOT NULL REFERENCES inventory_batches (id),
    ingredient_id           TEXT        NOT NULL REFERENCES ingredients (id),
    operation               TEXT        NOT NULL,
    quantity_change         REAL        NOT NULL,
    quantity_before         REAL        NOT NULL,
    quantity_after          REAL        NOT NULL,
    reason                  TEXT        NOT NULL,
    related_menu_id         TEXT        REFERENCES daily_menus (id),
    operator_id             TEXT        NOT NULL REFERENCES users (id),
    snapshot_recipe_amount  REAL,
    snapshot_servings       REAL,
    snapshot_recipe_name    TEXT,
    snapshot_ingredient_name TEXT,
    created_at              TIMESTAMPTZ NOT NULL,
    updated_at              TIMESTAMPTZ NOT NULL
);
CREATE INDEX IF NOT EXISTS idx_il_batch_id       ON inventory_logs (batch_id);
CREATE INDEX IF NOT EXISTS idx_il_family_created ON inventory_logs (family_id, created_at);

-- ============================================================================
-- 16. shopping_list_items
-- ============================================================================
CREATE TABLE IF NOT EXISTS shopping_list_items (
    id                  TEXT        PRIMARY KEY,
    family_id           TEXT        NOT NULL REFERENCES families (id),
    ingredient_id       TEXT        NOT NULL REFERENCES ingredients (id),
    required_quantity   REAL        NOT NULL,
    is_purchased        BOOLEAN     DEFAULT FALSE,
    purchased_at        TIMESTAMPTZ,
    source_menu_id      TEXT        REFERENCES daily_menus (id),
    created_at          TIMESTAMPTZ NOT NULL,
    updated_at          TIMESTAMPTZ NOT NULL
);
CREATE INDEX IF NOT EXISTS idx_sli_family_id    ON shopping_list_items (family_id);
CREATE INDEX IF NOT EXISTS idx_sli_is_purchased ON shopping_list_items (is_purchased);

-- ============================================================================
-- 17. consumption_records
-- ============================================================================
CREATE TABLE IF NOT EXISTS consumption_records (
    id              TEXT        PRIMARY KEY,
    user_id         TEXT        NOT NULL REFERENCES users (id),
    family_id       TEXT        NOT NULL REFERENCES families (id),
    date            DATE        NOT NULL,
    meal_type       TEXT        NOT NULL,
    recipe_id       TEXT        REFERENCES recipes (id),
    servings        REAL        DEFAULT 1.0,
    calories        REAL,
    protein         REAL,
    carbs           REAL,
    fat             REAL,
    menu_item_id    TEXT        REFERENCES menu_items (id),
    is_deleted      BOOLEAN     DEFAULT FALSE,
    version         INTEGER     DEFAULT 1,
    created_at      TIMESTAMPTZ NOT NULL,
    updated_at      TIMESTAMPTZ NOT NULL
);
CREATE INDEX IF NOT EXISTS idx_cr_user_date   ON consumption_records (user_id, date);
CREATE INDEX IF NOT EXISTS idx_cr_family_date ON consumption_records (family_id, date);

-- ============================================================================
-- 18. taste_preferences
-- ============================================================================
CREATE TABLE IF NOT EXISTS taste_preferences (
    id          TEXT        PRIMARY KEY,
    user_id     TEXT        NOT NULL UNIQUE REFERENCES users (id),
    salty       REAL        DEFAULT 0.5,
    sweet       REAL        DEFAULT 0.5,
    sour        REAL        DEFAULT 0.5,
    spicy       REAL        DEFAULT 0.5,
    umami       REAL        DEFAULT 0.5,
    version     INTEGER     DEFAULT 1,
    updated_at  TIMESTAMPTZ NOT NULL
);

-- ============================================================================
-- 19. ingredient_blacklist
-- ============================================================================
CREATE TABLE IF NOT EXISTS ingredient_blacklist (
    id              TEXT        PRIMARY KEY,
    user_id         TEXT        NOT NULL REFERENCES users (id),
    ingredient_id   TEXT        NOT NULL REFERENCES ingredients (id),
    created_at      TIMESTAMPTZ NOT NULL,
    UNIQUE (user_id, ingredient_id)
);

-- ============================================================================
-- 20. user_allergens
-- ============================================================================
CREATE TABLE IF NOT EXISTS user_allergens (
    id              TEXT        PRIMARY KEY,
    user_id         TEXT        NOT NULL REFERENCES users (id),
    ingredient_id   TEXT        NOT NULL REFERENCES ingredients (id),
    severity        TEXT        DEFAULT 'mild',
    created_at      TIMESTAMPTZ NOT NULL,
    UNIQUE (user_id, ingredient_id)
);

-- ============================================================================
-- 21. nutrition_profiles
-- ============================================================================
CREATE TABLE IF NOT EXISTS nutrition_profiles (
    id                      TEXT        PRIMARY KEY,
    user_id                 TEXT        NOT NULL UNIQUE REFERENCES users (id),
    daily_calorie_target    REAL,
    calorie_deficit         REAL        DEFAULT 0,
    protein_ratio           REAL        DEFAULT 0.30,
    carbs_ratio             REAL        DEFAULT 0.45,
    fat_ratio               REAL        DEFAULT 0.25,
    reminder_interval       INTEGER     DEFAULT 14,
    next_reminder_at        TIMESTAMPTZ,
    version                 INTEGER     DEFAULT 1,
    updated_at              TIMESTAMPTZ NOT NULL
);

-- ============================================================================
-- 22. health_reports
-- ============================================================================
CREATE TABLE IF NOT EXISTS health_reports (
    id              TEXT        PRIMARY KEY,
    family_id       TEXT        NOT NULL REFERENCES families (id),
    user_id         TEXT        REFERENCES users (id),
    report_type     TEXT        NOT NULL,
    report_scope    TEXT        NOT NULL,
    start_date      DATE        NOT NULL,
    end_date        DATE        NOT NULL,
    report_data     JSONB       NOT NULL,   -- PostgreSQL 专用 JSONB 类型
    generated_at    TIMESTAMPTZ NOT NULL
);
CREATE INDEX IF NOT EXISTS idx_hr_family_id ON health_reports (family_id);
CREATE INDEX IF NOT EXISTS idx_hr_user_id   ON health_reports (user_id);
CREATE INDEX IF NOT EXISTS idx_hr_period    ON health_reports (start_date, end_date);

-- ============================================================================
-- sync_queue 表仅在本地 SQLite 中存在，云端不需要
-- ============================================================================
