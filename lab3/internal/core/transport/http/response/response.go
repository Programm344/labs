package response

import (
	"encoding/json"
	"net/http"

	"lab3-rbac/internal/core/logger"
	"go.uber.org/zap"
)

type ResponseHandler struct {
	log *logger.Logger
	w   http.ResponseWriter
}

func NewHTTPResponseHandler(log *logger.Logger, w http.ResponseWriter) *ResponseHandler {
	return &ResponseHandler{log: log, w: w}
}

func (h *ResponseHandler) SuccessResponse(data interface{}) {
	h.writeJSON(http.StatusOK, data)
}

func (h *ResponseHandler) CreatedResponse(data interface{}) {
	h.writeJSON(http.StatusCreated, data)
}

func (h *ResponseHandler) NoContentResponse() {
	h.w.WriteHeader(http.StatusNoContent)
}

func (h *ResponseHandler) BadRequestResponse(message string) {
	h.writeJSON(http.StatusBadRequest, map[string]string{"error": message})
}

func (h *ResponseHandler) UnauthorizedResponse(message string) {
	h.writeJSON(http.StatusUnauthorized, map[string]string{"error": message})
}

func (h *ResponseHandler) ForbiddenResponse(message string) {
	h.writeJSON(http.StatusForbidden, map[string]string{"error": message})
}

func (h *ResponseHandler) InternalServerErrorResponse(message string) {
	h.writeJSON(http.StatusInternalServerError, map[string]string{"error": message})
}

func (h *ResponseHandler) writeJSON(status int, data interface{}) {
	h.w.Header().Set("Content-Type", "application/json")
	h.w.WriteHeader(status)
	if err := json.NewEncoder(h.w).Encode(data); err != nil {
		h.log.Error("failed to write JSON response", zap.Error(err))
	}
}
