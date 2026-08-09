# ─────────────────────────────────────────────────────
#  FluxDB Dashboard — Dockerfile (nginx static server)
# ─────────────────────────────────────────────────────

FROM nginx:1.25-alpine

# Copy dashboard files to nginx's default serve directory
COPY . /usr/share/nginx/html

# Nginx config: serve on 80, proxy /api to backend
COPY nginx.conf /etc/nginx/conf.d/default.conf

EXPOSE 80
