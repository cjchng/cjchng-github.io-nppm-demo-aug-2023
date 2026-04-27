#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Usage:
  tools/test_recipe_load.sh /absolute/path/to/recipe.cast [seconds]
  tools/test_recipe_load.sh --active [seconds]

Behavior:
  - launches x_mainmoil with DISPLAY set
  - interrupts it after the timeout
  - reports whether normal recipe loading markers were observed

Environment overrides:
  DISPLAY
  CJ_NPPM_META
  NPPM_BINARY
EOF
}

if [[ $# -lt 1 || $# -gt 2 ]]; then
  usage
  exit 2
fi

MODE="$1"
TIMEOUT_SECONDS="${2:-8}"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
BINARY="${NPPM_BINARY:-${REPO_ROOT}/build/apps/nppm/x_mainmoil}"
DISPLAY_VALUE="${DISPLAY:-:0}"

if [[ ! -x "${BINARY}" ]]; then
  echo "FAIL: binary not found or not executable: ${BINARY}" >&2
  exit 1
fi

RUN_ARGS=()
TARGET_DESC=""

if [[ "${MODE}" == "--active" ]]; then
  TARGET_DESC="active recipe flow"
else
  if [[ ! -f "${MODE}" ]]; then
    echo "FAIL: recipe file not found: ${MODE}" >&2
    exit 1
  fi
  RUN_ARGS=("${MODE}")
  TARGET_DESC="${MODE}"
fi

STAMP="$(date +%Y%m%d-%H%M%S)"
LOG_FILE="${REPO_ROOT}/tmp.recipe-load.${STAMP}.log"

before_born="$(find "${REPO_ROOT}" -maxdepth 1 -name 'born_*' -printf '%f\n' | sort || true)"
before_links="$(find "${REPO_ROOT}" -maxdepth 1 -name 'running_recipe@*' -printf '%f\n' | sort || true)"

echo "Recipe load test"
echo "binary: ${BINARY}"
echo "target: ${TARGET_DESC}"
echo "timeout: ${TIMEOUT_SECONDS}s"
echo "display: ${DISPLAY_VALUE}"
if [[ -n "${CJ_NPPM_META:-}" ]]; then
  echo "CJ_NPPM_META: ${CJ_NPPM_META}"
fi
echo

set +e
env DISPLAY="${DISPLAY_VALUE}" \
  timeout -s INT "${TIMEOUT_SECONDS}" \
  "${BINARY}" "${RUN_ARGS[@]}" >"${LOG_FILE}" 2>&1
cmd_status=$?
set -e

after_born="$(find "${REPO_ROOT}" -maxdepth 1 -name 'born_*' -printf '%f\n' | sort || true)"
after_links="$(find "${REPO_ROOT}" -maxdepth 1 -name 'running_recipe@*' -printf '%f\n' | sort || true)"

new_born="$(comm -13 <(printf '%s\n' "${before_born}") <(printf '%s\n' "${after_born}") | tail -1 || true)"
new_link="$(comm -13 <(printf '%s\n' "${before_links}") <(printf '%s\n' "${after_links}") | tail -1 || true)"

pass=true

if ! grep -q "md-> Mapping Init Ready!!" "${LOG_FILE}"; then
  pass=false
fi

if grep -Eq "Unable to locate a runtime recipe|Listed recipe file does not exist|Active recipe file points to a missing recipe|Format Error in recipe list|File of zero length .* or not existed" "${LOG_FILE}"; then
  pass=false
fi

echo "Result log: ${LOG_FILE}"
echo "Process exit code: ${cmd_status}"
echo "New running_recipe link: ${new_link:-<none>}"
echo "New born file: ${new_born:-<none>}"
echo
echo "Key lines:"
grep -E "Using recipe from|A 'cast' File is Selected|Active recipe file|Recipe list|Mapping Init Ready|Unable to locate a runtime recipe|Listed recipe file does not exist|Active recipe file points to a missing recipe|File of zero length" "${LOG_FILE}" || true
echo

if [[ -n "${new_link}" ]]; then
  echo "running_recipe target:"
  ls -l "${REPO_ROOT}/${new_link}" || true
  echo
fi

if [[ "${pass}" == true ]]; then
  echo "PASS: normal recipe loading markers were observed."
  exit 0
fi

echo "FAIL: normal recipe loading markers were not observed."
exit 1
