#!/bin/bash

NUM_PROCESSES=$1

if [ -z "$NUM_PROCESSES" ] || [ "$NUM_PROCESSES" -lt 2 ]; then
    echo "Usage: bash test_cluster.sh <number of processes (greater or equal to 2)>"
    exit 1
fi

SERVER="$(pwd)/server"
TMP_DIR=$(mktemp -d)

PIDS=()
REPLICA_PORTS=()

cleanup() {
    echo "cleaning up..."

    for pid in "${PIDS[@]}"; do
        kill "$pid" 2>/dev/null
    done

    rm -rf "$TMP_DIR"
}

trap cleanup EXIT INT TERM

for ((i = 1; i < NUM_PROCESSES; i++)); do
    PORT=$((6380 + i - 1))
    REPLICA_PORTS+=("$PORT")

    mkdir -p "$TMP_DIR/replica$i"

    (
        cd "$TMP_DIR/replica$i"
        "$SERVER" "$PORT" replica
    ) &

    PIDS+=($!)
done
sleep 1

PRIMARY_CMD=("$SERVER" 6379 primary)

for port in "${REPLICA_PORTS[@]}"; do
    PRIMARY_CMD+=("$port")
done

mkdir -p "$TMP_DIR/primary"

(
    cd "$TMP_DIR/primary"
    "${PRIMARY_CMD[@]}"
) &

PIDS+=($!)

wait