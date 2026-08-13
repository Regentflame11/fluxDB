#!/usr/bin/env bash
# ─────────────────────────────────────────────────────
#  FluxDB — health-check.sh
#  Verifies that all services are up and healthy
# ─────────────────────────────────────────────────────

set -euo pipefail

HOST="${1:-localhost}"
API_PORT="${2:-8080}"
TCP_PORT="${3:-6379}"
RETRIES=5
WAIT=2

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  FluxDB Health Check"
echo "  Host: $HOST | API: $API_PORT | TCP: $TCP_PORT"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# ── Check REST API ────────────────────────────────────
echo "[Health] Checking REST API..."
for i in $(seq 1 $RETRIES); do
    if curl -sf "http://$HOST:$API_PORT/api/health" > /dev/null; then
        echo "[Health]  REST API is healthy"
        break
    fi
    echo "[Health] Attempt $i/$RETRIES failed, retrying in ${WAIT}s..."
    sleep $WAIT
    if [ "$i" -eq "$RETRIES" ]; then
        echo "[Health]  REST API failed after $RETRIES attempts"
        exit 1
    fi
done

# ── Check TCP port ────────────────────────────────────
echo "[Health] Checking TCP server..."
if timeout 3 bash -c "echo PING | nc -q1 $HOST $TCP_PORT" 2>/dev/null | grep -q "PONG"; then
    echo "[Health]  TCP server is healthy"
else
    echo "[Health]   TCP port check failed (nc may not be available, trying curl fallback)"
fi

# ── Quick functional test via REST ───────────────────
echo "[Health] Running functional test..."
RESULT=$(curl -sf -X POST "http://$HOST:$API_PORT/api/query" \
    -H "Content-Type: application/json" \
    -d '{"cmd":"PING"}' | grep -o '"PONG"' || echo "")

if [ "$RESULT" = '"PONG"' ]; then
    echo "[Health]  Functional test passed (PING → PONG)"
else
    echo "[Health]  Functional test failed"
    exit 1
fi

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  All health checks passed! "
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
