CREATE TABLE IF NOT EXISTS roles (
    id BIGSERIAL PRIMARY KEY,
    name VARCHAR(255) NOT NULL UNIQUE,
    slug VARCHAR(255) NOT NULL UNIQUE,
    description TEXT,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    created_by BIGINT NOT NULL,
    updated_at TIMESTAMP,
    deleted_at TIMESTAMP,
    deleted_by BIGINT
);

CREATE INDEX idx_roles_slug ON roles(slug);
CREATE INDEX idx_roles_deleted_at ON roles(deleted_at);