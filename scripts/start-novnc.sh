#!/usr/bin/env bash
set -euo pipefail

export DISPLAY="${DISPLAY:-:1}"
PORT="${PORT:-7860}"
VNC_RESOLUTION="${VNC_RESOLUTION:-900x900}"
VNC_DEPTH="${VNC_DEPTH:-24}"

Xvfb "$DISPLAY" -screen 0 "${VNC_RESOLUTION}x${VNC_DEPTH}" -ac +extension GLX +render -noreset &
XVFB_PID=$!

fluxbox >/tmp/fluxbox.log 2>&1 &
x11vnc -display "$DISPLAY" -forever -shared -nopw -rfbport 5900 >/tmp/x11vnc.log 2>&1 &

cd /app
./pacman >/tmp/pacman.log 2>&1 &
PACMAN_PID=$!

cleanup() {
    kill "$PACMAN_PID" "$XVFB_PID" >/dev/null 2>&1 || true
}
trap cleanup EXIT

websockify --web=/usr/share/novnc/ "$PORT" localhost:5900
