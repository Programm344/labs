package middleware

import (
	"context"
	"net/http"
	"strings"

	"lab3-rbac/internal/core/auth"
	"lab3-rbac/internal/core/logger"
	"lab3-rbac/internal/core/transport/http/response"

	"go.uber.org/zap"
)

func AuthMiddleware(authService *auth.Service) Middleware {
	return func(next http.Handler) http.Handler {
		return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
			ctx := r.Context()
			log := logger.FromContext(ctx)
			rh := response.NewHTTPResponseHandler(log, w)

			authHeader := r.Header.Get("Authorization")
			if authHeader == "" {
				rh.UnauthorizedResponse("missing authorization header")
				return
			}

			parts := strings.SplitN(authHeader, " ", 2)
			if len(parts) != 2 || parts[0] != "Bearer" {
				rh.UnauthorizedResponse("invalid authorization header format")
				return
			}

			claims, err := authService.ValidateToken(parts[1])
			if err != nil {
				log.Warn("invalid token", zap.Error(err))
				rh.UnauthorizedResponse("invalid token")
				return
			}

			ctx = context.WithValue(ctx, "user_id", claims.UserID)
			ctx = context.WithValue(ctx, "email", claims.Email)
			next.ServeHTTP(w, r.WithContext(ctx))
		})
	}
}
