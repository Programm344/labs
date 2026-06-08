package permissions

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

func (s *Service) GetAll(ctx context.Context, page, limit int) ([]domain.Permission, int64, error) {
	var permissions []domain.Permission
	var total int64

	query := s.db.WithContext(ctx).Model(&domain.Permission{}).Where("deleted_at IS NULL")

	if err := query.Count(&total).Error; err != nil {
		return nil, 0, err
	}

	offset := (page - 1) * limit
	if err := query.Offset(offset).Limit(limit).Find(&permissions).Error; err != nil {
		return nil, 0, err
	}

	return permissions, total, nil
}

func (s *Service) GetByID(ctx context.Context, id uint64) (*domain.Permission, error) {
	var perm domain.Permission
	err := s.db.WithContext(ctx).Where("id = ? AND deleted_at IS NULL", id).First(&perm).Error
	if err != nil {
		return nil, errors.New("permission not found")
	}
	return &perm, nil
}

func (s *Service) Create(ctx context.Context, name, slug string, description *string, createdBy uint64) (*domain.Permission, error) {
	var existing domain.Permission
	if err := s.db.WithContext(ctx).Where("slug = ?", slug).First(&existing).Error; err == nil {
		return nil, errors.New("permission with this slug already exists")
	}

	perm := &domain.Permission{
		Name:        name,
		Slug:        slug,
		Description: description,
		CreatedBy:   createdBy,
		CreatedAt:   time.Now(),
	}

	if err := s.db.WithContext(ctx).Create(perm).Error; err != nil {
		return nil, err
	}

	return perm, nil
}

func (s *Service) Update(ctx context.Context, id uint64, name, slug string, description *string) (*domain.Permission, error) {
	var perm domain.Permission
	err := s.db.WithContext(ctx).Where("id = ? AND deleted_at IS NULL", id).First(&perm).Error
	if err != nil {
		return nil, errors.New("permission not found")
	}

	var existing domain.Permission
	if err := s.db.WithContext(ctx).Where("slug = ? AND id != ?", slug, id).First(&existing).Error; err == nil {
		return nil, errors.New("permission with this slug already exists")
	}

	now := time.Now()
	perm.Name = name
	perm.Slug = slug
	perm.Description = description
	perm.UpdatedAt = &now

	if err := s.db.WithContext(ctx).Save(&perm).Error; err != nil {
		return nil, err
	}

	return &perm, nil
}

func (s *Service) SoftDelete(ctx context.Context, id, deletedBy uint64) error {
	result := s.db.WithContext(ctx).
		Model(&domain.Permission{}).
		Where("id = ? AND deleted_at IS NULL", id).
		Updates(map[string]interface{}{
			"deleted_at": time.Now(),
			"deleted_by": deletedBy,
		})

	if result.Error != nil {
		return result.Error
	}
	if result.RowsAffected == 0 {
		return errors.New("permission not found or already deleted")
	}
	return nil
}

func (s *Service) HardDelete(ctx context.Context, id uint64) error {
	result := s.db.WithContext(ctx).Unscoped().Where("id = ?", id).Delete(&domain.Permission{})
	if result.Error != nil {
		return result.Error
	}
	if result.RowsAffected == 0 {
		return errors.New("permission not found")
	}
	return nil
}

func (s *Service) Restore(ctx context.Context, id uint64) (*domain.Permission, error) {
	var perm domain.Permission
	err := s.db.WithContext(ctx).Unscoped().Where("id = ? AND deleted_at IS NOT NULL", id).First(&perm).Error
	if err != nil {
		return nil, errors.New("deleted permission not found")
	}

	if err := s.db.WithContext(ctx).Model(&perm).Updates(map[string]interface{}{
		"deleted_at": nil,
		"deleted_by": nil,
	}).Error; err != nil {
		return nil, err
	}

	return &perm, nil
}
