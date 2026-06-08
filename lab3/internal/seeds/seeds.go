package seeds

import (
	"log"
	"time"

	"golang.org/x/crypto/bcrypt"
	"gorm.io/gorm"

	"lab3-rbac/internal/core/domain"
)

func RunAll(db *gorm.DB) error {
	adminID, err := createAdminUser(db)
	if err != nil {
		return err
	}

	roles, err := createRoles(db, adminID)
	if err != nil {
		return err
	}

	permissions, err := createPermissions(db, adminID)
	if err != nil {
		return err
	}

	if err := assignPermissionsToRoles(db, adminID, roles, permissions); err != nil {
		return err
	}

	if err := assignRolesToUsers(db, adminID, roles); err != nil {
		return err
	}

	log.Println("Seeds completed!")
	return nil
}

func createAdminUser(db *gorm.DB) (uint64, error) {
	hashedPassword, err := bcrypt.GenerateFromPassword([]byte("Admin123!"), bcrypt.DefaultCost)
	if err != nil {
		return 0, err
	}

	admin := domain.User{
		Email:        "admin@example.com",
		PasswordHash: string(hashedPassword),
		Name:         "Administrator",
		CreatedAt:    time.Now(),
	}

	var existing domain.User
	if err := db.Where("email = ?", admin.Email).First(&existing).Error; err == nil {
		log.Println("Admin user already exists")
		return existing.ID, nil
	}

	if err := db.Create(&admin).Error; err != nil {
		return 0, err
	}

	log.Printf("Created admin user: %d", admin.ID)
	return admin.ID, nil
}

func createRoles(db *gorm.DB, createdBy uint64) (map[string]*domain.Role, error) {
	rolesData := []struct {
		Name, Slug, Desc string
	}{
		{"Administrator", "admin", "Full system access"},
		{"User", "user", "Regular user access"},
		{"Guest", "guest", "Limited read-only access"},
	}

	roles := make(map[string]*domain.Role)

	for _, rd := range rolesData {
		var role domain.Role
		err := db.Where("slug = ?", rd.Slug).First(&role).Error

		if err == gorm.ErrRecordNotFound {
			desc := rd.Desc
			role = domain.Role{
				Name:        rd.Name,
				Slug:        rd.Slug,
				Description: &desc,
				CreatedBy:   createdBy,
				CreatedAt:   time.Now(),
			}
			if err := db.Create(&role).Error; err != nil {
				return nil, err
			}
			log.Printf("Created role: %s", rd.Slug)
		}

		roles[rd.Slug] = &role
	}

	return roles, nil
}

func createPermissions(db *gorm.DB, createdBy uint64) (map[string]*domain.Permission, error) {
	entities := []string{"user", "role", "permission"}
	actions := []string{"get-list", "read", "create", "update", "delete", "restore"}

	permissions := make(map[string]*domain.Permission)

	for _, entity := range entities {
		for _, action := range actions {
			slug := action + "-" + entity
			name := action + " " + entity

			var perm domain.Permission
			err := db.Where("slug = ?", slug).First(&perm).Error

			if err == gorm.ErrRecordNotFound {
				desc := "Allows to " + action + " " + entity
				perm = domain.Permission{
					Name:        name,
					Slug:        slug,
					Description: &desc,
					CreatedBy:   createdBy,
					CreatedAt:   time.Now(),
				}
				if err := db.Create(&perm).Error; err != nil {
					return nil, err
				}
				log.Printf("Created permission: %s", slug)
			}

			permissions[slug] = &perm
		}
	}

	return permissions, nil
}

func assignPermissionsToRoles(db *gorm.DB, createdBy uint64, roles map[string]*domain.Role, permissions map[string]*domain.Permission) error {
	// Admin получает все права
	for _, perm := range permissions {
		if err := db.Create(&domain.PermissionRole{
			RoleID:       roles["admin"].ID,
			PermissionID: perm.ID,
			CreatedBy:    createdBy,
			CreatedAt:    time.Now(),
		}).Error; err != nil {
			// Игнорируем дубликаты
		}
	}

	// User: чтение и обновление пользователей
	userPerms := []string{"get-list-user", "read-user", "update-user"}
	for _, slug := range userPerms {
		db.Create(&domain.PermissionRole{
			RoleID:       roles["user"].ID,
			PermissionID: permissions[slug].ID,
			CreatedBy:    createdBy,
			CreatedAt:    time.Now(),
		})
	}

	// Guest: только чтение списка пользователей
	db.Create(&domain.PermissionRole{
		RoleID:       roles["guest"].ID,
		PermissionID: permissions["get-list-user"].ID,
		CreatedBy:    createdBy,
		CreatedAt:    time.Now(),
	})

	return nil
}

func assignRolesToUsers(db *gorm.DB, createdBy uint64, roles map[string]*domain.Role) error {
	var adminUser domain.User
	if err := db.Where("email = ?", "admin@example.com").First(&adminUser).Error; err != nil {
		return err
	}

	return db.Create(&domain.RoleUser{
		UserID:    adminUser.ID,
		RoleID:    roles["admin"].ID,
		CreatedBy: createdBy,
		CreatedAt: time.Now(),
	}).Error
}
