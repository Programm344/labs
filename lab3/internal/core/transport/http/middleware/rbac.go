package middleware

import (
	"net/http"

	"lab3-rbac/internal/core/auth"
	"lab3-rbac/internal/core/logger"
	"lab3-rbac/internal/core/transport/http/response"

	"go.uber.org/zap"
)

func RBACMiddleware(authService *auth.Service, permission string) Middleware {
	return func(next http.Handler) http.Handler {
		return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
			ctx := r.Context()
			log := logger.FromContext(ctx)
			rh := response.NewHTTPResponseHandler(log, w)

			userID, ok := ctx.Value("user_id").(uint64)
			if !ok {
				rh.UnauthorizedResponse("authorization required")
				return
			}

			hasPermission, err := authService.UserHasPermission(userID, permission)
			if err != nil {
				log.Error("failed to check permission", zap.Error(err))
				rh.InternalServerErrorResponse("failed to check permissions")
				return
			}

			if !hasPermission {
				log.Warn("access denied",
					zap.Uint64("user_id", userID),
					zap.String("required_permission", permission),
				)
				rh.ForbiddenResponse("Access denied. Required permission: " + permission)
				return
			}

			next.ServeHTTP(w, r)
		})
	}
}
