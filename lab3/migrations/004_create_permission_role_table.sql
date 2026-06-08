CREATE TABLE IF NOT EXISTS permission_role (
    id BIGSERIAL PRIMARY KEY,
    role_id BIGINT NOT NULL REFERENCES roles(id) ON DELETE CASCADE,
    permission_id BIGINT NOT NULL REFERENCES permissions(id) ON DELETE CASCADE,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    created_by BIGINT NOT NULL,
    deleted_at TIMESTAMP,
    deleted_by BIGINT,
    UNIQUE(role_id, permission_id, deleted_at)
);

CREATE INDEX idx_permission_role_role_id ON permission_role(role_id);
CREATE INDEX idx_permission_role_permission_id ON permission_role(permission_id);