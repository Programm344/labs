package http

import "lab3-rbac/internal/core/domain"

type UserDTO struct {
	ID    uint64 `json:"id"`
	Email string `json:"email"`
	Name  string `json:"name"`
}

func ToUserDTO(u *domain.User) *UserDTO {
	return &UserDTO{
		ID:    u.ID,
		Email: u.Email,
		Name:  u.Name,
	}
}
