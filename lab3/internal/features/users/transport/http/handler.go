package http

import (
	"encoding/json"
	"lab3-rbac/internal/core/logger"
	"lab3-rbac/internal/core/transport/http/response"
	"lab3-rbac/internal/features/users"
	"net/http"
	"strconv"

	"go.uber.org/zap"
)

type UserHandler struct {
	service *users.Service
}

func NewUserHandler(service *users.Service) *UserHandler {
	return &UserHandler{service: service}
}

// GetUsers список пользователей
func (h *UserHandler) GetUsers() http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		ctx := r.Context()
		log := logger.FromContext(ctx)
		rh := response.NewHTTPResponseHandler(log, w)

		page, _ := strconv.Atoi(r.URL.Query().Get("page"))
		limit, _ := strconv.Atoi(r.URL.Query().Get("limit"))
		if page < 1 {
			page = 1
		}
		if limit < 1 || limit > 100 {
			limit = 10
		}

		users, total, err := h.service.GetAll(ctx, page, limit)
		if err != nil {
			log.Error("failed to get users", zap.Error(err))
			rh.InternalServerErrorResponse("failed to get users")
			return
		}

		result := make([]*UserDTO, len(users))
		for i := range users {
			result[i] = ToUserDTO(&users[i])
		}

		rh.SuccessResponse(map[string]interface{}{
			"data": result,
			"meta": map[string]interface{}{"total": total, "page": page, "limit": limit},
		})
	}
}

// GetUserRoles роли пользователя
func (h *UserHandler) GetUserRoles() http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		ctx := r.Context()
		log := logger.FromContext(ctx)
		rh := response.NewHTTPResponseHandler(log, w)

		userID, err := strconv.ParseUint(r.PathValue("id"), 10, 64)
		if err != nil {
			rh.BadRequestResponse("invalid user id")
			return
		}

		roles, err := h.service.GetRoles(ctx, userID)
		if err != nil {
			log.Error("failed to get user roles", zap.Error(err))
			rh.InternalServerErrorResponse("failed to get user roles")
			return
		}

		rh.SuccessResponse(roles)
	}
}

// AttachRole назначить роль
func (h *UserHandler) AttachRole() http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		ctx := r.Context()
		log := logger.FromContext(ctx)
		rh := response.NewHTTPResponseHandler(log, w)

		userID, _ := strconv.ParseUint(r.PathValue("id"), 10, 64)
		createdBy, _ := ctx.Value("user_id").(uint64)

		var req struct {
			RoleID uint64 `json:"role_id"`
		}
		if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
			rh.BadRequestResponse("invalid request body")
			return
		}

		if err := h.service.AttachRole(ctx, userID, req.RoleID, createdBy); err != nil {
			log.Error("failed to attach role", zap.Error(err))
			rh.BadRequestResponse(err.Error())
			return
		}

		rh.NoContentResponse()
	}
}

// DetachRole удалить роль
func (h *UserHandler) DetachRole() http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		ctx := r.Context()
		rh := response.NewHTTPResponseHandler(logger.FromContext(ctx), w)

		userID, _ := strconv.ParseUint(r.PathValue("id"), 10, 64)
		roleID, _ := strconv.ParseUint(r.PathValue("role_id"), 10, 64)
		deletedBy, _ := ctx.Value("user_id").(uint64)

		if err := h.service.DetachRole(ctx, userID, roleID, deletedBy, false); err != nil {
			rh.BadRequestResponse(err.Error())
			return
		}
		rh.NoContentResponse()
	}
}

// SoftDetachRole мягкое удаление роли
func (h *UserHandler) SoftDetachRole() http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		ctx := r.Context()
		rh := response.NewHTTPResponseHandler(logger.FromContext(ctx), w)

		userID, _ := strconv.ParseUint(r.PathValue("id"), 10, 64)
		roleID, _ := strconv.ParseUint(r.PathValue("role_id"), 10, 64)
		deletedBy, _ := ctx.Value("user_id").(uint64)

		if err := h.service.DetachRole(ctx, userID, roleID, deletedBy, true); err != nil {
			rh.BadRequestResponse(err.Error())
			return
		}
		rh.NoContentResponse()
	}
}

// RestoreRole восстановить роль
func (h *UserHandler) RestoreRole() http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		ctx := r.Context()
		rh := response.NewHTTPResponseHandler(logger.FromContext(ctx), w)

		userID, _ := strconv.ParseUint(r.PathValue("id"), 10, 64)
		roleID, _ := strconv.ParseUint(r.PathValue("role_id"), 10, 64)

		if err := h.service.RestoreRole(ctx, userID, roleID); err != nil {
			rh.BadRequestResponse(err.Error())
			return
		}
		rh.NoContentResponse()
	}
}
