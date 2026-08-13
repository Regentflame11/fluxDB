#!/usr/bin/env bash
# ─────────────────────────────────────────────────────
#  FluxDB — deploy.sh
#  Pulls latest code and redeploys on the EC2 server
#
#  Usage (run locally):
#    ./scripts/deploy.sh <EC2_HOST> <SSH_KEY_PATH>
#
#  Usage (GitHub Actions — uses secrets):
#    Called automatically by ci-cd.yml
# ─────────────────────────────────────────────────────

set -euo pipefail

EC2_HOST="${1:-$EC2_HOST}"
SSH_KEY="${2:-$SSH_KEY_PATH}"
APP_DIR="${APP_DIR:-/home/ubuntu/fluxdb}"
BRANCH="${BRANCH:-main}"

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  FluxDB Deployment Script"
echo "  Host:   $EC2_HOST"
echo "  Branch: $BRANCH"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# SSH into EC2 and run deployment commands
ssh -i "$SSH_KEY" \
    -o StrictHostKeyChecking=no \
    ubuntu@"$EC2_HOST" << EOF

    set -euo pipefail

    echo "[Deploy] Navigating to app directory..."
    cd $APP_DIR || { echo "ERROR: $APP_DIR not found. Run setup first."; exit 1; }

    echo "[Deploy] Pulling latest code from git..."
    git fetch origin
    git checkout $BRANCH
    git pull origin $BRANCH

    echo "[Deploy] Stopping old containers..."
    docker compose down --remove-orphans || true

    echo "[Deploy] Building and starting containers..."
    docker compose build --no-cache
    docker compose up -d

    echo "[Deploy] Waiting for health check..."
    sleep 5
    ./scripts/health-check.sh

    echo "[Deploy]  Deployment successful!"
    echo "[Deploy] Dashboard: http://$EC2_HOST:3000"
    echo "[Deploy] TCP:       $EC2_HOST:6379"

EOF
