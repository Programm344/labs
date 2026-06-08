package http

import (
	"time"

	"lab3-rbac/internal/core/domain"
)

type CreateRoleRequest struct {
	Name        string  `json:"name"`
	Slug        string  `json:"slug"`
	Description *string `json:"description,omitempty"`
}

type RoleDTO struct {
	ID          uint64    `json:"id"`
	Name        string    `json:"name"`
	Slug        string    `json:"slug"`
	Description *string   `json:"description,omitempty"`
	CreatedAt   time.Time `json:"created_at"`
}
type UpdateRoleRequest struct {
	Name        string  `json:"name"`
	Slug        string  `json:"slug"`
	Description *string `json:"description,omitempty"`
}

func ToRoleDTO(role *domain.Role) *RoleDTO {
	return &RoleDTO{
		ID:          role.ID,
		Name:        role.Name,
		Slug:        role.Slug,
		Description: role.Description,
		CreatedAt:   role.CreatedAt,
	}
}
