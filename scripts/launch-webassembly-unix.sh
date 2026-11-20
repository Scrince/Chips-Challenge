#!/bin/sh
set -eu

package_directory=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cd "$package_directory"

if ! command -v python3 >/dev/null 2>&1; then
    echo "Python 3 is required to serve the WebAssembly game." >&2
    exit 1
fi

case "$(uname -s)" in
    Darwin)
        if ! command -v open >/dev/null 2>&1; then
            echo "The macOS 'open' command is not available." >&2
            exit 1
        fi
        open_browser() { open "$1"; }
        ;;
    Linux*)
        if ! command -v xdg-open >/dev/null 2>&1; then
            echo "Install xdg-utils to open the game in your default browser." >&2
            exit 1
        fi
        open_browser() { xdg-open "$1"; }
        ;;
    *)
        echo "This launcher supports Linux and macOS." >&2
        exit 1
        ;;
esac

port=$(python3 -c 'import socket; s=socket.socket(); s.bind(("127.0.0.1", 0)); print(s.getsockname()[1]); s.close()')
python3 -m http.server "$port" --bind 127.0.0.1 &
server_pid=$!
cleanup() {
    kill "$server_pid" 2>/dev/null || true
}
trap cleanup EXIT HUP INT TERM

url="http://127.0.0.1:$port/"
ready=false
attempt=0
while [ "$attempt" -lt 40 ]; do
    if ! kill -0 "$server_pid" 2>/dev/null; then
        echo "The local web server exited before it was ready." >&2
        exit 1
    fi
    if python3 -c 'import sys, urllib.request; r=urllib.request.urlopen(sys.argv[1], timeout=1); sys.exit(0 if r.status == 200 and b"chips-native.js" in r.read() else 1)' "$url" 2>/dev/null; then
        ready=true
        break
    fi
    attempt=$((attempt + 1))
    sleep 0.25
done

if [ "$ready" != true ]; then
    echo "The local web server did not become ready in time." >&2
    exit 1
fi

open_browser "$url"
echo "Chip's Challenge is open in your default browser."
echo "Press Ctrl+C in this terminal to stop the local server."
wait "$server_pid"
