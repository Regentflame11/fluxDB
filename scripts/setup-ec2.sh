#!/usr/bin/env bash
# ─────────────────────────────────────────────────────
#  FluxDB — EC2 Setup Script
#  Run this ONCE on a fresh Ubuntu 22.04 EC2 instance
#  Usage: bash setup-ec2.sh
# ─────────────────────────────────────────────────────
set -euo pipefail

echo "===================================="
echo "  FluxDB EC2 Setup"
echo "===================================="

# ── 1. Update system ─────────────────────────────────
echo "[1/5] Updating system packages..."
sudo apt-get update -qq
sudo apt-get upgrade -y -qq

# ── 2. Install Docker ─────────────────────────────────
echo "[2/5] Installing Docker..."
sudo apt-get install -y -qq ca-certificates curl gnupg

sudo install -m 0755 -d /etc/apt/keyrings
curl -fsSL https://download.docker.com/linux/ubuntu/gpg \
    | sudo gpg --dearmor -o /etc/apt/keyrings/docker.gpg
sudo chmod a+r /etc/apt/keyrings/docker.gpg

echo "deb [arch=$(dpkg --print-architecture) \
  signed-by=/etc/apt/keyrings/docker.gpg] \
  https://download.docker.com/linux/ubuntu \
  $(. /etc/os-release && echo "$VERSION_CODENAME") stable" \
  | sudo tee /etc/apt/sources.list.d/docker.list > /dev/null

sudo apt-get update -qq
sudo apt-get install -y -qq docker-ce docker-ce-cli containerd.io docker-compose-plugin

# Allow ubuntu user to run docker without sudo
sudo usermod -aG docker ubuntu
sudo systemctl enable docker
sudo systemctl start docker

echo "[2/5] Docker installed."

# ── 3. Install Git ────────────────────────────────────
echo "[3/5] Installing Git..."
sudo apt-get install -y -qq git

# ── 4. Clone FluxDB repo ──────────────────────────────
echo "[4/5] Cloning FluxDB..."
cd /home/ubuntu
if [ -d "fluxDB" ]; then
    echo "Repo already exists, pulling latest..."
    cd fluxDB && git pull origin main
else
    git clone https://github.com/Regentflame11/fluxDB.git
    cd fluxDB
fi

# ── 5. Build and run with Docker Compose ──────────────
echo "[5/5] Building and starting FluxDB..."
sudo docker compose build
sudo docker compose up -d

# ── Done ──────────────────────────────────────────────
sleep 3
PUBLIC_IP=$(curl -sf http://169.254.169.254/latest/meta-data/public-ipv4 || echo "your-ec2-ip")

echo ""
echo "===================================="
echo "  FluxDB is LIVE"
echo "===================================="
echo "  Dashboard : http://$PUBLIC_IP:3000"
echo "  REST API  : http://$PUBLIC_IP:8080/api/stats"
echo "  TCP       : $PUBLIC_IP:6379"
echo "===================================="
echo ""
echo "To stop:  docker compose down"
echo "To start: docker compose up -d"
