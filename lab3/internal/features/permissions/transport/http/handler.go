package http

import (
	"encoding/json"
	"net/http"
	"strconv"

	"lab3-rbac/internal/core/logger"
	"lab3-rbac/internal/core/transport/http/response"
	"lab3-rbac/internal/features/permissions"

	"go.uber.org/zap"
)

type PermissionHandler struct {
	service *permissions.Service
}

func NewPermissionHandler(service *permissions.Service) *PermissionHandler {
	return &PermissionHandler{service: service}
}

func (h *PermissionHandler) GetPermissions() http.HandlerFunc {
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

		perms, total, err := h.service.GetAll(ctx, page, limit)
		if err != nil {
			log.Error("failed to get permissions", zap.Error(err))
			rh.InternalServerErrorResponse("failed to get permissions")
			return
		}

		result := make([]*PermissionDTO, len(perms))
		for i := range perms {
			result[i] = ToPermissionDTO(&perms[i])
		}

		rh.SuccessResponse(map[string]interface{}{
			"data": result,
			"meta": map[string]interface{}{"total": total, "page": page, "limit": limit},
		})
	}
}

func (h *PermissionHandler) GetPermission() http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		ctx := r.Context()
		log := logger.FromContext(ctx)
		rh := response.NewHTTPResponseHandler(log, w)

		id, _ := strconv.ParseUint(r.PathValue("id"), 10, 64)
		perm, err := h.service.GetByID(ctx, id)
		if err != nil {
			rh.BadRequestResponse(err.Error())
			return
		}
		rh.SuccessResponse(ToPermissionDTO(perm))
	}
}

func (h *PermissionHandler) CreatePermission() http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		ctx := r.Context()
		log := logger.FromContext(ctx)
		rh := response.NewHTTPResponseHandler(log, w)

		var req CreatePermissionRequest
		if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
			rh.BadRequestResponse("invalid request body")
			return
		}
		if req.Name == "" || req.Slug == "" {
			rh.BadRequestResponse("name and slug are required")
			return
		}

		userID, _ := ctx.Value("user_id").(uint64)
		perm, err := h.service.Create(ctx, req.Name, req.Slug, req.Description, userID)
		if err != nil {
			rh.BadRequestResponse(err.Error())
			return
		}
		rh.CreatedResponse(ToPermissionDTO(perm))
	}
}

func (h *PermissionHandler) UpdatePermission() http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		ctx := r.Context()
		rh := response.NewHTTPResponseHandler(logger.FromContext(ctx), w)

		id, _ := strconv.ParseUint(r.PathValue("id"), 10, 64)
		var req UpdatePermissionRequest
		json.NewDecoder(r.Body).Decode(&req)
		if req.Name == "" || req.Slug == "" {
			rh.BadRequestResponse("name and slug are required")
			return
		}

		perm, err := h.service.Update(ctx, id, req.Name, req.Slug, req.Description)
		if err != nil {
			rh.BadRequestResponse(err.Error())
			return
		}
		rh.SuccessResponse(ToPermissionDTO(perm))
	}
}

func (h *PermissionHandler) SoftDeletePermission() http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		ctx := r.Context()
		rh := response.NewHTTPResponseHandler(logger.FromContext(ctx), w)

		id, _ := strconv.ParseUint(r.PathValue("id"), 10, 64)
		userID, _ := ctx.Value("user_id").(uint64)
		if err := h.service.SoftDelete(ctx, id, userID); err != nil {
			rh.BadRequestResponse(err.Error())
			return
		}
		rh.NoContentResponse()
	}
}

func (h *PermissionHandler) HardDeletePermission() http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		ctx := r.Context()
		rh := response.NewHTTPResponseHandler(logger.FromContext(ctx), w)

		id, _ := strconv.ParseUint(r.PathValue("id"), 10, 64)
		if err := h.service.HardDelete(ctx, id); err != nil {
			rh.BadRequestResponse(err.Error())
			return
		}
		rh.NoContentResponse()
	}
}

func (h *PermissionHandler) RestorePermission() http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		ctx := r.Context()
		rh := response.NewHTTPResponseHandler(logger.FromContext(ctx), w)

		id, _ := strconv.ParseUint(r.PathValue("id"), 10, 64)
		perm, err := h.service.Restore(ctx, id)
		if err != nil {
			rh.BadRequestResponse(err.Error())
			return
		}
		rh.SuccessResponse(ToPermissionDTO(perm))
	}
}
