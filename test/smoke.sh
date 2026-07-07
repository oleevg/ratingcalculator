#!/usr/bin/env bash
# Smoke test: start server + client for a few seconds and verify neither crashes.
# Usage: smoke.sh <service_binary> <client_binary>

SERVICE="${1:?Error: service binary path required as \$1}"
CLIENT="${2:?Error: client binary path required as \$2}"

RUN_DURATION=5    # seconds both processes must stay alive
PORT_WAIT=10      # seconds to wait for the server to open its port
PASS=0
FAIL=0

# Pick a free TCP port by letting the OS assign one, then freeing it.
free_port() {
    python3 -c "import socket; s=socket.socket(); s.bind(('',0)); print(s.getsockname()[1]); s.close()"
}

run_smoke() {
    local transport="$1"
    local port="$2"
    local server_pid client_pid

    echo "--- ${transport} on port ${port} ---"

    "$SERVICE" --transport "$transport" -p "$port" -t 3 --threads 2 >/dev/null 2>&1 &
    server_pid=$!

    # Wait until the server port accepts connections
    local deadline=$(( SECONDS + PORT_WAIT ))
    until nc -z localhost "$port" 2>/dev/null; do
        if (( SECONDS >= deadline )); then
            echo "FAIL: ${transport} server did not open port ${port} within ${PORT_WAIT}s"
            kill "$server_pid" 2>/dev/null; wait "$server_pid" 2>/dev/null || true
            FAIL=$(( FAIL + 1 ))
            return
        fi
        sleep 0.2
    done

    if ! kill -0 "$server_pid" 2>/dev/null; then
        echo "FAIL: ${transport} server exited before client connected"
        FAIL=$(( FAIL + 1 ))
        return
    fi

    "$CLIENT" --transport "$transport" -a localhost -p "$port" -u 10 -t 1 >/dev/null 2>&1 &
    client_pid=$!

    sleep "$RUN_DURATION"

    local ok=1
    if ! kill -0 "$server_pid" 2>/dev/null; then
        echo "FAIL: ${transport} server crashed within ${RUN_DURATION}s"
        ok=0
    fi
    if ! kill -0 "$client_pid" 2>/dev/null; then
        echo "FAIL: ${transport} client crashed within ${RUN_DURATION}s"
        ok=0
    fi

    kill "$server_pid" "$client_pid" 2>/dev/null
    wait "$server_pid" "$client_pid" 2>/dev/null || true

    if (( ok )); then
        echo "PASS: ${transport} — both processes alive after ${RUN_DURATION}s"
        PASS=$(( PASS + 1 ))
    else
        FAIL=$(( FAIL + 1 ))
    fi
}

run_smoke ws    "$(free_port)"
run_smoke grpc  "$(free_port)"

echo ""
echo "Results: ${PASS} passed, ${FAIL} failed"
(( FAIL == 0 ))
