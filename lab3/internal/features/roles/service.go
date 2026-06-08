package roles

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

func (s *Service) GetAll(ctx context.Context, page, limit int) ([]domain.Role, int64, error) {
	var roles []domain.Role
	var total int64

	query := s.db.WithContext(ctx).Model(&domain.Role{}).Where("deleted_at IS NULL")

	if err := query.Count(&total).Error; err != nil {
		return nil, 0, err
	}

	offset := (page - 1) * limit
	if err := query.Offset(offset).Limit(limit).Find(&roles).Error; err != nil {
		return nil, 0, err
	}

	return roles, total, nil
}

func (s *Service) Create(ctx context.Context, name, slug string, description *string, createdBy uint64) (*domain.Role, error) {
	var existing domain.Role
	if err := s.db.WithContext(ctx).Where("slug = ?", slug).First(&existing).Error; err == nil {
		return nil, errors.New("role with this slug already exists")
	}

	role := &domain.Role{
		Name:        name,
		Slug:        slug,
		Description: description,
		CreatedBy:   createdBy,
		CreatedAt:   time.Now(),
	}

	if err := s.db.WithContext(ctx).Create(role).Error; err != nil {
		return nil, err
	}

	return role, nil
}

func (s *Service) SoftDelete(ctx context.Context, id, deletedBy uint64) error {
	result := s.db.WithContext(ctx).
		Model(&domain.Role{}).
		Where("id = ? AND deleted_at IS NULL", id).
		Updates(map[string]interface{}{
			"deleted_at": time.Now(),
			"deleted_by": deletedBy,
		})

	if result.Error != nil {
		return result.Error
	}
	if result.RowsAffected == 0 {
		return errors.New("role not found or already deleted")
	}

	return nil
}
func (s *Service) GetByID(ctx context.Context, id uint64) (*domain.Role, error) {
	var role domain.Role
	err := s.db.WithContext(ctx).Where("id = ? AND deleted_at IS NULL", id).First(&role).Error
	if err != nil {
		if errors.Is(err, gorm.ErrRecordNotFound) {
			return nil, errors.New("role not found")
		}
		return nil, err
	}
	return &role, nil
}

func (s *Service) Update(ctx context.Context, id uint64, name, slug string, description *string) (*domain.Role, error) {
	var role domain.Role
	err := s.db.WithContext(ctx).Where("id = ? AND deleted_at IS NULL", id).First(&role).Error
	if err != nil {
		return nil, errors.New("role not found")
	}

	// Проверяем уникальность slug (кроме себя)
	var existing domain.Role
	if err := s.db.WithContext(ctx).Where("slug = ? AND id != ?", slug, id).First(&existing).Error; err == nil {
		return nil, errors.New("role with this slug already exists")
	}

	now := time.Now()
	role.Name = name
	role.Slug = slug
	role.Description = description
	role.UpdatedAt = &now

	if err := s.db.WithContext(ctx).Save(&role).Error; err != nil {
		return nil, err
	}

	return &role, nil
}

func (s *Service) HardDelete(ctx context.Context, id uint64) error {
	result := s.db.WithContext(ctx).Unscoped().Where("id = ?", id).Delete(&domain.Role{})
	if result.Error != nil {
		return result.Error
	}
	if result.RowsAffected == 0 {
		return errors.New("role not found")
	}
	return nil
}

func (s *Service) Restore(ctx context.Context, id uint64) (*domain.Role, error) {
	var role domain.Role
	err := s.db.WithContext(ctx).Unscoped().Where("id = ? AND deleted_at IS NOT NULL", id).First(&role).Error
	if err != nil {
		return nil, errors.New("deleted role not found")
	}

	if err := s.db.WithContext(ctx).Model(&role).Updates(map[string]interface{}{
		"deleted_at": nil,
		"deleted_by": nil,
	}).Error; err != nil {
		return nil, err
	}

	role.DeletedAt = nil
	role.DeletedBy = nil
	return &role, nil
}
