package middleware

import (
	"net/http"
	"time"

	"lab3-rbac/internal/core/logger"
	"go.uber.org/zap"
)

type responseWriter struct {
	http.ResponseWriter
	statusCode int
}

func (rw *responseWriter) WriteHeader(code int) {
	rw.statusCode = code
	rw.ResponseWriter.WriteHeader(code)
}

func Trace() Middleware {
	return func(next http.Handler) http.Handler {
		return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
			ctx := r.Context()
			log := logger.FromContext(ctx)
			rw := &responseWriter{ResponseWriter: w, statusCode: http.StatusOK}
			before := time.Now()

			log.Debug(">>> incoming HTTP request",
				zap.Time("time", before.UTC()),
			)

			next.ServeHTTP(rw, r)

			log.Debug("<<< done HTTP request",
				zap.Int("status_code", rw.statusCode),
				zap.Duration("latency", time.Since(before)),
			)
		})
	}
}
