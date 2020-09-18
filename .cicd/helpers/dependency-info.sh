#!/bin/bash
set -eo pipefail
[[ "$RAW_PIPELINE_CONFIG" == '' ]] && export RAW_PIPELINE_CONFIG="$1"
[[ "$RAW_PIPELINE_CONFIG" == '' ]] && export RAW_PIPELINE_CONFIG='pipeline.jsonc'
[[ "$PIPELINE_CONFIG" == '' ]] && export PIPELINE_CONFIG='pipeline.json'
# read dependency file
if [[ -f "$RAW_PIPELINE_CONFIG" ]]; then
    echo 'Reading pipeline configuration file...'
    cat "$RAW_PIPELINE_CONFIG" | grep -Po '^[^"/]*("((?<=\\).|[^"])*"[^"/]*)*' | jq -c .\"eosio-dot-contracts\" > "$PIPELINE_CONFIG"
    CDT_VERSION=$(cat "$PIPELINE_CONFIG" | jq -r '.dependencies."eosio.cdt"')
    EOSIO_VERSION=$(cat "$PIPELINE_CONFIG" | jq -r '.dependencies.eosio')
    SANITIZED_EOSIO_VERSION=$(echo $EOSIO_VERSION | sed 's/\//\_/')
else
    echo 'ERROR: No pipeline configuration file or dependencies file found!'
    exit 1
fi
# search GitHub for commit hash by tag and branch, preferring tag if both match
if [[ "$BUILDKITE" == 'true' ]]; then
    CDT_COMMIT=$((curl -s https://api.github.com/repos/EOSIO/eosio.cdt/git/refs/tags/$CDT_VERSION && curl -s https://api.github.com/repos/EOSIO/eosio.