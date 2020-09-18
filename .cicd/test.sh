#!/bin/bash
set -eo pipefail
. ./.cicd/helpers/buildkite.sh
. ./.cicd/helpers/general.sh
mkdir -p $BUILD_DIR
if [[ "$BUILDKITE" == 'true'