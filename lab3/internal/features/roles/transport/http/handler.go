package http

import (
	"encoding/json"
	"net/http"
	"strconv"

	"lab3-rbac/internal/core/logger"
	"lab3-rbac/internal/core/transport/http/response"
	"lab3-rbac/internal/features/roles"

	"go.uber.org/zap"
)

type RoleHandler struct {
	roleService *roles.Service
}

func NewRoleHandler(roleService *roles.Service) *RoleHandler {
	return &RoleHandler{roleService: roleService}
}

func (h *RoleHandler) GetRoles() http.HandlerFunc {
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

		roles, total, err := h.roleService.GetAll(ctx, page, limit)
		if err != nil {
			log.Error("failed to get roles", zap.Error(err))
			rh.InternalServerErrorResponse("failed to get roles")
			return
		}

		result := make([]*RoleDTO, len(roles))
		for i := range roles {
			result[i] = ToRoleDTO(&roles[i])
		}

		rh.SuccessResponse(map[string]interface{}{
			"data": result,
			"meta": map[string]interface{}{
				"total": total,
				"page":  page,
				"limit": limit,
			},
		})
	}
}

func (h *RoleHandler) CreateRole() http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		ctx := r.Context()
		log := logger.FromContext(ctx)
		rh := response.NewHTTPResponseHandler(log, w)

		var req CreateRoleRequest
		if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
			rh.BadRequestResponse("invalid request body")
			return
		}

		if req.Name == "" || req.Slug == "" {
			rh.BadRequestResponse("name and slug are required")
			return
		}

		userID, _ := ctx.Value("user_id").(uint64)

		role, err := h.roleService.Create(ctx, req.Name, req.Slug, req.Description, userID)
		if err != nil {
			log.Error("failed to create role", zap.Error(err))
			rh.BadRequestResponse(err.Error())
			return
		}

		rh.CreatedResponse(ToRoleDTO(role))
	}
}

func (h *RoleHandler) SoftDeleteRole() http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		ctx := r.Context()
		log := logger.FromContext(ctx)
		rh := response.NewHTTPResponseHandler(log, w)

		id, err := strconv.ParseUint(r.PathValue("id"), 10, 64)
		if err != nil {
			rh.BadRequestResponse("invalid role id")
			return
		}

		userID, _ := ctx.Value("user_id").(uint64)

		if err := h.roleService.SoftDelete(ctx, id, userID); err != nil {
			log.Error("failed to delete role", zap.Error(err))
			rh.BadRequestResponse(err.Error())
			return
		}

		rh.NoContentResponse()
	}
}
func (h *RoleHandler) GetRole() http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		ctx := r.Context()
		log := logger.FromContext(ctx)
		rh := response.NewHTTPResponseHandler(log, w)

		id, err := strconv.ParseUint(r.PathValue("id"), 10, 64)
		if err != nil {
			rh.BadRequestResponse("invalid role id")
			return
		}

		role, err := h.roleService.GetByID(ctx, id)
		if err != nil {
			rh.BadRequestResponse(err.Error())
			return
		}

		rh.SuccessResponse(ToRoleDTO(role))
	}
}

func (h *RoleHandler) UpdateRole() http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		ctx := r.Context()
		log := logger.FromContext(ctx)
		rh := response.NewHTTPResponseHandler(log, w)

		id, err := strconv.ParseUint(r.PathValue("id"), 10, 64)
		if err != nil {
			rh.BadRequestResponse("invalid role id")
			return
		}

		var req UpdateRoleRequest
		if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
			rh.BadRequestResponse("invalid request body")
			return
		}

		if req.Name == "" || req.Slug == "" {
			rh.BadRequestResponse("name and slug are required")
			return
		}

		role, err := h.roleService.Update(ctx, id, req.Name, req.Slug, req.Description)
		if err != nil {
			log.Error("failed to update role", zap.Error(err))
			rh.BadRequestResponse(err.Error())
			return
		}

		rh.SuccessResponse(ToRoleDTO(role))
	}
}

func (h *RoleHandler) HardDeleteRole() http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		ctx := r.Context()
		log := logger.FromContext(ctx)
		rh := response.NewHTTPResponseHandler(log, w)

		id, err := strconv.ParseUint(r.PathValue("id"), 10, 64)
		if err != nil {
			rh.BadRequestResponse("invalid role id")
			return
		}

		if err := h.roleService.HardDelete(ctx, id); err != nil {
			log.Error("failed to delete role", zap.Error(err))
			rh.BadRequestResponse(err.Error())
			return
		}

		rh.NoContentResponse()
	}
}

func (h *RoleHandler) RestoreRole() http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		ctx := r.Context()
		log := logger.FromContext(ctx)
		rh := response.NewHTTPResponseHandler(log, w)

		id, err := strconv.ParseUint(r.PathValue("id"), 10, 64)
		if err != nil {
			rh.BadRequestResponse("invalid role id")
			return
		}

		role, err := h.roleService.Restore(ctx, id)
		if err != nil {
			log.Error("failed to restore role", zap.Error(err))
			rh.BadRequestResponse(err.Error())
			return
		}

		rh.SuccessResponse(ToRoleDTO(role))
	}
}
