package main

import (
	"encoding/json"
	"fmt"
	"log"
	"net/http"
	"os"
	"os/signal"
	"syscall"
	"time"

	"lab3-rbac/internal/core/auth"
	"lab3-rbac/internal/core/logger"
	"lab3-rbac/internal/core/transport/http/middleware"
	"lab3-rbac/internal/features/permissions"
	permissionsHttp "lab3-rbac/internal/features/permissions/transport/http"
	"lab3-rbac/internal/features/roles"
	rolesHttp "lab3-rbac/internal/features/roles/transport/http"
	"lab3-rbac/internal/features/users"
	usersHttp "lab3-rbac/internal/features/users/transport/http"
	"lab3-rbac/internal/seeds"

	"github.com/joho/godotenv"
	"github.com/kelseyhightower/envconfig"
	"go.uber.org/zap"
	"go.uber.org/zap/zapcore"
	"gorm.io/driver/postgres"
	"gorm.io/gorm"
)

type Config struct {
	Addr            string        `envconfig:"ADDR" required:"true"`
	ShutdownTimeout time.Duration `envconfig:"SHUTDOWN_TIMEOUT" required:"true"`
}

func main() {
	if err := godotenv.Load(); err != nil {
		log.Println("No .env file found")
	}

	var cfg Config
	if err := envconfig.Process("HTTP", &cfg); err != nil {
		log.Fatal(err)
	}

	dbCfg := struct {
		Host     string
		Port     string
		User     string
		Password string
		Name     string
	}{
		Host:     os.Getenv("POSTGRES_HOST"),
		Port:     os.Getenv("POSTGRES_PORT"),
		User:     os.Getenv("POSTGRES_USER"),
		Password: os.Getenv("POSTGRES_PASSWORD"),
		Name:     os.Getenv("POSTGRES_DB"),
	}

	appLogger := logger.New(zapcore.DebugLevel)
	appLogger.Info("Starting RBAC application")

	dsn := fmt.Sprintf("host=%s port=%s user=%s password=%s dbname=%s sslmode=disable",
		dbCfg.Host, dbCfg.Port, dbCfg.User, dbCfg.Password, dbCfg.Name)

	db, err := gorm.Open(postgres.Open(dsn), &gorm.Config{})
	if err != nil {
		appLogger.Error("Failed to connect to database", zap.Error(err))
		os.Exit(1)
	}
	appLogger.Info("Database connected")

	// Запускаем seeds
	if err := seeds.RunAll(db); err != nil {
		appLogger.Error("Seeds failed", zap.Error(err))
	}

	// Сервисы
	roleService := roles.NewService(db)
	jwtSecret := os.Getenv("JWT_SECRET")
	authService := auth.NewService(db, jwtSecret)
	permService := permissions.NewService(db)
	permHandler := permissionsHttp.NewPermissionHandler(permService)
	// Handlers
	roleHandler := rolesHttp.NewRoleHandler(roleService)
	userService := users.NewService(db)
	userHandler := usersHttp.NewUserHandler(userService)
	// Основной роутер
	mux := http.NewServeMux()

	// Публичный эндпоинт — логин
	mux.HandleFunc("POST /api/v1/login", func(w http.ResponseWriter, r *http.Request) {
		var req struct {
			Email    string `json:"email"`
			Password string `json:"password"`
		}
		json.NewDecoder(r.Body).Decode(&req)

		token, err := authService.Login(req.Email, req.Password)
		if err != nil {
			w.Header().Set("Content-Type", "application/json")
			w.WriteHeader(http.StatusUnauthorized)
			json.NewEncoder(w).Encode(map[string]string{"error": "invalid credentials"})
			return
		}

		w.Header().Set("Content-Type", "application/json")
		json.NewEncoder(w).Encode(map[string]string{"token": token})
	})

	// Защищённые эндпоинты (требуют авторизацию)
	protectedMux := http.NewServeMux()
	// Защищённые эндпоинты с проверкой прав
	protectedMux.Handle("GET /api/v1/ref/user", middleware.ChainMiddleware(
		http.HandlerFunc(userHandler.GetUsers()),
		middleware.RBACMiddleware(authService, "get-list-user"),
	))
	protectedMux.Handle("GET /api/v1/ref/user/{id}/role", middleware.ChainMiddleware(
		http.HandlerFunc(userHandler.GetUserRoles()),
		middleware.RBACMiddleware(authService, "read-user"),
	))
	protectedMux.Handle("POST /api/v1/ref/user/{id}/role", middleware.ChainMiddleware(
		http.HandlerFunc(userHandler.AttachRole()),
		middleware.RBACMiddleware(authService, "update-user"),
	))
	protectedMux.Handle("DELETE /api/v1/ref/user/{id}/role/{role_id}", middleware.ChainMiddleware(
		http.HandlerFunc(userHandler.DetachRole()),
		middleware.RBACMiddleware(authService, "update-user"),
	))
	protectedMux.Handle("DELETE /api/v1/ref/user/{id}/role/{role_id}/soft", middleware.ChainMiddleware(
		http.HandlerFunc(userHandler.SoftDetachRole()),
		middleware.RBACMiddleware(authService, "update-user"),
	))
	protectedMux.Handle("POST /api/v1/ref/user/{id}/role/{role_id}/restore", middleware.ChainMiddleware(
		http.HandlerFunc(userHandler.RestoreRole()),
		middleware.RBACMiddleware(authService, "update-user"),
	))
	protectedMux.Handle("GET /api/v1/ref/policy/permission", middleware.ChainMiddleware(
		http.HandlerFunc(permHandler.GetPermissions()),
		middleware.RBACMiddleware(authService, "get-list-permission"),
	))
	protectedMux.Handle("GET /api/v1/ref/policy/permission/{id}", middleware.ChainMiddleware(
		http.HandlerFunc(permHandler.GetPermission()),
		middleware.RBACMiddleware(authService, "read-permission"),
	))
	protectedMux.Handle("POST /api/v1/ref/policy/permission", middleware.ChainMiddleware(
		http.HandlerFunc(permHandler.CreatePermission()),
		middleware.RBACMiddleware(authService, "create-permission"),
	))
	protectedMux.Handle("PUT /api/v1/ref/policy/permission/{id}", middleware.ChainMiddleware(
		http.HandlerFunc(permHandler.UpdatePermission()),
		middleware.RBACMiddleware(authService, "update-permission"),
	))
	protectedMux.Handle("DELETE /api/v1/ref/policy/permission/{id}/soft", middleware.ChainMiddleware(
		http.HandlerFunc(permHandler.SoftDeletePermission()),
		middleware.RBACMiddleware(authService, "delete-permission"),
	))
	protectedMux.Handle("DELETE /api/v1/ref/policy/permission/{id}", middleware.ChainMiddleware(
		http.HandlerFunc(permHandler.HardDeletePermission()),
		middleware.RBACMiddleware(authService, "delete-permission"),
	))
	protectedMux.Handle("POST /api/v1/ref/policy/permission/{id}/restore", middleware.ChainMiddleware(
		http.HandlerFunc(permHandler.RestorePermission()),
		middleware.RBACMiddleware(authService, "restore-permission"),
	))
	protectedMux.Handle("GET /api/v1/ref/policy/role", middleware.ChainMiddleware(
		http.HandlerFunc(roleHandler.GetRoles()),
		middleware.RBACMiddleware(authService, "get-list-role"),
	))

	protectedMux.Handle("POST /api/v1/ref/policy/role", middleware.ChainMiddleware(
		http.HandlerFunc(roleHandler.CreateRole()),
		middleware.RBACMiddleware(authService, "create-role"),
	))

	protectedMux.Handle("DELETE /api/v1/ref/policy/role/{id}/soft", middleware.ChainMiddleware(
		http.HandlerFunc(roleHandler.SoftDeleteRole()),
		middleware.RBACMiddleware(authService, "delete-role"),
	))
	protectedHandler := middleware.ChainMiddleware(
		protectedMux,
		middleware.AuthMiddleware(authService),
		middleware.RequestId(),
		middleware.LoggerMiddleware(appLogger),
		middleware.Panic(),
		middleware.Trace(),
	)
	// Роли
	protectedMux.Handle("GET /api/v1/ref/policy/role/{id}", middleware.ChainMiddleware(
		http.HandlerFunc(roleHandler.GetRole()),
		middleware.RBACMiddleware(authService, "read-role"),
	))
	protectedMux.Handle("PUT /api/v1/ref/policy/role/{id}", middleware.ChainMiddleware(
		http.HandlerFunc(roleHandler.UpdateRole()),
		middleware.RBACMiddleware(authService, "update-role"),
	))
	protectedMux.Handle("DELETE /api/v1/ref/policy/role/{id}", middleware.ChainMiddleware(
		http.HandlerFunc(roleHandler.HardDeleteRole()),
		middleware.RBACMiddleware(authService, "delete-role"),
	))
	protectedMux.Handle("POST /api/v1/ref/policy/role/{id}/restore", middleware.ChainMiddleware(
		http.HandlerFunc(roleHandler.RestoreRole()),
		middleware.RBACMiddleware(authService, "restore-role"),
	))
	mux.Handle("/api/v1/ref/", protectedHandler)

	// Общие middleware для всех
	handler := middleware.ChainMiddleware(
		mux,
		middleware.RequestId(),
		middleware.LoggerMiddleware(appLogger),
		middleware.Panic(),
		middleware.Trace(),
	)

	server := &http.Server{
		Addr:    cfg.Addr,
		Handler: handler,
	}

	go func() {
		appLogger.Info("Server starting", zap.String("addr", cfg.Addr))
		if err := server.ListenAndServe(); err != nil && err != http.ErrServerClosed {
			appLogger.Error("Server failed", zap.Error(err))
			os.Exit(1)
		}
	}()

	quit := make(chan os.Signal, 1)
	signal.Notify(quit, syscall.SIGINT, syscall.SIGTERM)
	<-quit

	appLogger.Info("Shutting down server...")
	appLogger.Info("Server exited gracefully")
}
