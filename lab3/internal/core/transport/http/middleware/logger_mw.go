package middleware

import (
	"net/http"

	"lab3-rbac/internal/core/logger"
	"go.uber.org/zap"
)

func LoggerMiddleware(log *logger.Logger) Middleware {
	return func(next http.Handler) http.Handler {
		return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
			requestID := r.Header.Get(requestIDHeader)
			l := &logger.Logger{Logger: log.With(
				zap.String("request_id", requestID),
				zap.String("url", r.URL.String()),
			)}
			ctx := logger.ToContext(r.Context(), l)
			next.ServeHTTP(w, r.WithContext(ctx))
		})
	}
}
