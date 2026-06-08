package auth

import (
	"errors"
	"time"

	"github.com/golang-jwt/jwt/v5"
	"golang.org/x/crypto/bcrypt"
	"gorm.io/gorm"

	"lab3-rbac/internal/core/domain"
)

type Service struct {
	db        *gorm.DB
	jwtSecret []byte
}

func NewService(db *gorm.DB, jwtSecret string) *Service {
	return &Service{db: db, jwtSecret: []byte(jwtSecret)}
}

type Claims struct {
	UserID uint64 `json:"user_id"`
	Email  string `json:"email"`
	jwt.RegisteredClaims
}

func (s *Service) Login(email, password string) (string, error) {
	var user domain.User
	err := s.db.Where("email = ? AND deleted_at IS NULL", email).First(&user).Error
	if err != nil {
		return "", errors.New("invalid credentials")
	}

	err = bcrypt.CompareHashAndPassword([]byte(user.PasswordHash), []byte(password))
	if err != nil {
		return "", errors.New("invalid credentials")
	}

	claims := &Claims{
		UserID: user.ID,
		Email:  user.Email,
		RegisteredClaims: jwt.RegisteredClaims{
			ExpiresAt: jwt.NewNumericDate(time.Now().Add(24 * time.Hour)),
			IssuedAt:  jwt.NewNumericDate(time.Now()),
		},
	}

	token := jwt.NewWithClaims(jwt.SigningMethodHS256, claims)
	return token.SignedString(s.jwtSecret)
}
func (s *Service) UserHasPermission(userID uint64, permissionSlug string) (bool, error) {
	var count int64
	err := s.db.Table("role_user").
		Joins("JOIN permission_role ON permission_role.role_id = role_user.role_id").
		Joins("JOIN permissions ON permissions.id = permission_role.permission_id").
		Where("role_user.user_id = ? AND role_user.deleted_at IS NULL", userID).
		Where("permission_role.deleted_at IS NULL").
		Where("permissions.slug = ? AND permissions.deleted_at IS NULL", permissionSlug).
		Count(&count).Error

	return count > 0, err
}
func (s *Service) ValidateToken(tokenString string) (*Claims, error) {
	token, err := jwt.ParseWithClaims(tokenString, &Claims{}, func(token *jwt.Token) (interface{}, error) {
		return s.jwtSecret, nil
	})
	if err != nil {
		return nil, err
	}

	claims, ok := token.Claims.(*Claims)
	if !ok || !token.Valid {
		return nil, errors.New("invalid token")
	}

	return claims, nil
}
