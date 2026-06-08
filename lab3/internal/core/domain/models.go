package domain

import "time"

type User struct {
	ID           uint64    `gorm:"primaryKey"`
	Email        string    `gorm:"unique;not null"`
	PasswordHash string    `gorm:"not null"`
	Name         string    `gorm:"not null"`
	CreatedAt    time.Time `gorm:"not null"`
	UpdatedAt    *time.Time
	DeletedAt    *time.Time `gorm:"index"`
	Version      uint64     `gorm:"not null;default:1"`
}

func (User) TableName() string { return "users" }

type Role struct {
	ID          uint64 `gorm:"primaryKey"`
	Name        string `gorm:"unique;not null"`
	Slug        string `gorm:"unique;not null"`
	Description *string
	CreatedAt   time.Time `gorm:"not null"`
	CreatedBy   uint64    `gorm:"not null"`
	UpdatedAt   *time.Time
	DeletedAt   *time.Time `gorm:"index"`
	DeletedBy   *uint64
}

func (Role) TableName() string { return "roles" }

type Permission struct {
	ID          uint64 `gorm:"primaryKey"`
	Name        string `gorm:"unique;not null"`
	Slug        string `gorm:"unique;not null"`
	Description *string
	CreatedAt   time.Time `gorm:"not null"`
	CreatedBy   uint64    `gorm:"not null"`
	UpdatedAt   *time.Time
	DeletedAt   *time.Time `gorm:"index"`
	DeletedBy   *uint64
}

func (Permission) TableName() string { return "permissions" }

type RoleUser struct {
	ID        uint64     `gorm:"primaryKey"`
	UserID    uint64     `gorm:"not null;index"`
	RoleID    uint64     `gorm:"not null;index"`
	CreatedAt time.Time  `gorm:"not null"`
	CreatedBy uint64     `gorm:"not null"`
	DeletedAt *time.Time `gorm:"index"`
	DeletedBy *uint64
}

func (RoleUser) TableName() string { return "role_user" }

type PermissionRole struct {
	ID           uint64     `gorm:"primaryKey"`
	RoleID       uint64     `gorm:"not null;index"`
	PermissionID uint64     `gorm:"not null;index"`
	CreatedAt    time.Time  `gorm:"not null"`
	CreatedBy    uint64     `gorm:"not null"`
	DeletedAt    *time.Time `gorm:"index"`
	DeletedBy    *uint64
}

func (PermissionRole) TableName() string { return "permission_role" }
