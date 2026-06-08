package middleware

import (
	"net/http"

	"lab3-rbac/internal/core/logger"
	"lab3-rbac/internal/core/transport/http/response"
)

func Panic() Middleware {
	return func(next http.Handler) http.Handler {
		return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
			ctx := r.Context()
			log := logger.FromContext(ctx)
			rh := response.NewHTTPResponseHandler(log, w)

			defer func() {
				if p := recover(); p != nil {
					rh.InternalServerErrorResponse("internal server error")
				}
			}()
			next.ServeHTTP(w, r)
		})
	}
}
