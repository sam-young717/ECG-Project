set -euo pipefail

# This script should run AI Agent for C/C++test Static Analysis with AI Autofix.
# It is intended to be executed by 'cpptest-autofix-github.yml' but can also be run manually.

COPILOT_BIN="/home/syoung/.local/bin/copilot"
# Override at the job level by exporting CPPTEST_MCP_BIN before running this script.
CPPTEST_MCP_BIN="${CPPTEST_MCP_BIN:-/mnt/c/Users/syoung/Documents/Solutions/Linux/2026.1/Standard/cpptest/integration/mcp/cpptestmcp}"

# Sanity-check the MCP server binary up front so failures are visible in the job log
# (otherwise Copilot will just report "MCP tool unavailable" with no detail).
if [ ! -x "$CPPTEST_MCP_BIN" ]; then
  echo "ERROR: cpptest MCP binary not found or not executable at: $CPPTEST_MCP_BIN" >&2
  ls -l "$CPPTEST_MCP_BIN" >&2 || true
  exit 1
fi

# Re-register every run so the path can never drift from a stale config
"$COPILOT_BIN" mcp remove cpptest-std-mcp >/dev/null 2>&1 || true
"$COPILOT_BIN" mcp add cpptest-std-mcp -- "$CPPTEST_MCP_BIN"

echo "Registered MCP servers:"
"$COPILOT_BIN" mcp list || true

# Diagnostic: report which Copilot auth source is available. The CLI accepts a
# token via COPILOT_GITHUB_TOKEN, GH_TOKEN, or GITHUB_TOKEN, OR a cached login
# under $HOME/.copilot/ (created by running 'copilot' and using /login once).
# This block does NOT exit on missing env vars — the cached login is a valid
# fallback. If Copilot truly cannot authenticate, the CLI itself will error out
# on the next call with a clear message.
echo "Copilot auth diagnostics:"
echo "  HOME=$HOME"
for v in COPILOT_GITHUB_TOKEN GH_TOKEN GITHUB_TOKEN; do
  val="${!v:-}"
  if [ -n "$val" ]; then
    echo "  $v: set (${#val} chars)"
  else
    echo "  $v: empty/unset"
  fi
done
if [ -f "$HOME/.copilot/config.json" ]; then
  echo "  cached login: $HOME/.copilot/config.json exists"
else
  echo "  cached login: not found"
fi

# Execute the prompt with Copilot.
#   --allow-all-tools           auto-approve every tool (required for -p mode)
#   --allow-tool='cpptest-std-mcp'  explicitly trust our MCP server
#   --no-ask-user               agent works autonomously, never blocks waiting for input
#   --log-level debug           write detailed logs to $HOME/.copilot/logs/ for post-mortem
#   --enable-reasoning-summaries  surface the model's reasoning in stdout
#   --add-dir "$PWD"            ensure the workspace path is explicitly on the allowlist
"$COPILOT_BIN" \
  --allow-all-tools \
  --allow-tool='cpptest-std-mcp' \
  --no-ask-user \
  --log-level debug \
  --enable-reasoning-summaries \
  --add-dir "$PWD" \
  -p "$(cat "$(dirname "$0")/cpptest-agent-prompt.md")"