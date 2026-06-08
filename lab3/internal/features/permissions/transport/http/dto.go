package http

import (
	"time"

	"lab3-rbac/internal/core/domain"
)

type CreatePermissionRequest struct {
	Name        string  `json:"name"`
	Slug        string  `json:"slug"`
	Description *string `json:"description,omitempty"`
}

type UpdatePermissionRequest struct {
	Name        string  `json:"name"`
	Slug        string  `json:"slug"`
	Description *string `json:"description,omitempty"`
}

type PermissionDTO struct {
	ID          uint64    `json:"id"`
	Name        string    `json:"name"`
	Slug        string    `json:"slug"`
	Description *string   `json:"description,omitempty"`
	CreatedAt   time.Time `json:"created_at"`
}

func ToPermissionDTO(p *domain.Permission) *PermissionDTO {
	return &PermissionDTO{
		ID:          p.ID,
		Name:        p.Name,
		Slug:        p.Slug,
		Description: p.Description,
		CreatedAt:   p.CreatedAt,
	}
}
