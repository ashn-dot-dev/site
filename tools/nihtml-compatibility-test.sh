#!/bin/sh
set -e
export LC_ALL=C.UTF-8

PROGNAME=$(basename "$0")
usage() {
    cat <<EOF
usage: ${PROGNAME} [FILE...]

options:
  -h    Display usage information and exit.
EOF
}

for arg in "$@"; do
case "${arg}" in
    -h)
        usage
        exit 0
        ;;
    *)
        ;;
esac
done

if [ -x "${MELLIFERA_HOME}/bin/mf" ]; then
    MELLIFERA_PROG="${MELLIFERA_HOME}/bin/mf"
else
    MELLIFERA_PROG="${MELLIFERA_HOME}/mf.py"
fi


TESTSRUN=0
FAILURES=0

test() {
    TEST="$1"

    echo "[= TEST ${TEST} =]"

    set +e
    MF_OUTPUT=$(cat "${TEST}" | "${MELLIFERA_PROG}" nihtml.mf 2>&1)
    PY_OUTPUT=$(cat "${TEST}" | python3 nihtml.py 2>&1)
    set -e

    if [ "${MF_OUTPUT}" = "${PY_OUTPUT}" ]; then
        echo '[= PASS =]'
    else
        TMPDIR=$(mktemp -d)
        trap '{ rm -rf -- "${TMPDIR}"; }' EXIT
        printf '%s\n' "${MF_OUTPUT}" >"${TMPDIR}/mf.output"
        printf '%s\n' "${PY_OUTPUT}" >"${TMPDIR}/py.output"
        diff "${TMPDIR}/mf.output" "${TMPDIR}/py.output" || true
        echo '[= FAIL =]'
        FAILURES=$((FAILURES + 1))
    fi
    TESTSRUN=$((TESTSRUN + 1))
}

TESTS= # empty
if [ "$#" -ne 0 ]; then
    for arg in "$@"; do
        if [ -d "$(realpath ${arg})" ]; then
            FILES=$(find "$(realpath ${arg})" -name '*.md' | sort)
            TESTS=$(echo "${TESTS}" "${FILES}")
        else
            TESTS=$(echo "${TESTS}" "${arg}")
        fi
    done
else
    TESTS=$(find . -name '*.md' | sort)
fi

for t in ${TESTS}; do
    test "${t}"
done

echo "TESTS RUN => ${TESTSRUN}"
echo "FAILURES  => ${FAILURES}"

[ "${FAILURES}" -eq 0 ] || exit 1
