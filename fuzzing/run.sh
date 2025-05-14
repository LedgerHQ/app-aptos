#!/usr/bin/env bash

set -e

SCRIPTDIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
BUILDDIR="$SCRIPTDIR/build"
CORPUSDIR="$SCRIPTDIR/corpus"

"$BUILDDIR"/fuzz_tx_parser "$CORPUSDIR" "$@" >/dev/null
