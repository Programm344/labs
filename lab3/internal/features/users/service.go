package users

import (
	"context"
	"errors"
	"time"

	"lab3-rbac/internal/core/domain"

	"gorm.io/gorm"
)

type Service struct {
	db *gorm.DB
}

func NewService(db *gorm.DB) *Service {
	return &Service{db: db}
}

func (s *Service) GetAll(ctx context.Context, page, limit int) ([]domain.User, int64, error) {
	var users []domain.User
	var total int64

	query := s.db.WithContext(ctx).Model(&domain.User{}).Where("deleted_at IS NULL")

	if err := query.Count(&total).Error; err != nil {
		return nil, 0, err
	}

	offset := (page - 1) * limit
	if err := query.Offset(offset).Limit(limit).Find(&users).Error; err != nil {
		return nil, 0, err
	}

	return users, total, nil
}

func (s *Service) GetRoles(ctx context.Context, userID uint64) ([]domain.Role, error) {
	var roles []domain.Role
	err := s.db.WithContext(ctx).
		Joins("JOIN role_user ON role_user.role_id = roles.id").
		Where("role_user.user_id = ? AND role_user.deleted_at IS NULL", userID).
		Where("roles.deleted_at IS NULL").
		Find(&roles).Error

	return roles, err
}

func (s *Service) AttachRole(ctx context.Context, userID, roleID, createdBy uint64) error {
	var user domain.User
	if err := s.db.WithContext(ctx).Where("id = ? AND deleted_at IS NULL", userID).First(&user).Error; err != nil {
		return errors.New("user not found")
	}

	var role domain.Role
	if err := s.db.WithContext(ctx).Where("id = ? AND deleted_at IS NULL", roleID).First(&role).Error; err != nil {
		return errors.New("role not found")
	}

	var existing domain.RoleUser
	err := s.db.WithContext(ctx).Where("user_id = ? AND role_id = ? AND deleted_at IS NULL", userID, roleID).First(&existing).Error
	if err == nil {
		return errors.New("role already assigned")
	}

	err = s.db.WithContext(ctx).Unscoped().Where("user_id = ? AND role_id = ? AND deleted_at IS NOT NULL", userID, roleID).First(&existing).Error
	if err == nil {
		return s.db.WithContext(ctx).Model(&existing).Updates(map[string]interface{}{
			"deleted_at": nil,
			"deleted_by": nil,
		}).Error
	}

	roleUser := &domain.RoleUser{
		UserID:    userID,
		RoleID:    roleID,
		CreatedBy: createdBy,
		CreatedAt: time.Now(),
	}

	return s.db.WithContext(ctx).Create(roleUser).Error
}

func (s *Service) DetachRole(ctx context.Context, userID, roleID, deletedBy uint64, soft bool) error {
	if soft {
		return s.db.WithContext(ctx).
			Model(&domain.RoleUser{}).
			Where("user_id = ? AND role_id = ? AND deleted_at IS NULL", userID, roleID).
			Updates(map[string]interface{}{
				"deleted_at": time.Now(),
				"deleted_by": deletedBy,
			}).Error
	}

	return s.db.WithContext(ctx).Unscoped().
		Where("user_id = ? AND role_id = ?", userID, roleID).
		Delete(&domain.RoleUser{}).Error
}

func (s *Service) RestoreRole(ctx context.Context, userID, roleID uint64) error {
	return s.db.WithContext(ctx).Unscoped().
		Model(&domain.RoleUser{}).
		Where("user_id = ? AND role_id = ? AND deleted_at IS NOT NULL", userID, roleID).
		Updates(map[string]interface{}{
			"deleted_at": nil,
			"deleted_by": nil,
		}).Error
}
