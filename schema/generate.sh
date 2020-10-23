#!/usr/bin/env bash

# Really ugly and temporary glue to run moo code generator.
# This will simplify and move into CMake.

mydir=$(dirname $(realpath $BASH_SOURCE))
srcdir=$(dirname $mydir)

# Wrap up the render command.  This bakes in a mapping to file name
# which would be better somehow captured by the schema itself.
render () {
    local name="$1" ; shift
    local What="$1" ; shift
    local outdir="${1:-$srcdir/include/appfwk/${name}}"
    local what="$(echo $What | tr '[:upper:]' '[:lower:]')"
    local tmpl="appfwk-${what}.hpp.j2"
    local outhpp="$outdir/${What}.hpp"
    mkdir -p $outdir
    set -x
    moo -g '/lang:ocpp.jsonnet' \
        -M $mydir -T $mydir \
        -A path="dunedaq.appfwk.${name}" \
        -A ctxpath="dunedaq" \
        -A os="appfwk-${name}-schema.jsonnet" \
        render appfwk-model.jsonnet $tmpl \
        > $outhpp || exit -1
    set +x
    echo $outhpp
}


render cmd Structs
render cmd Nljs

render fdp Structs $srcdir/test/appfwk/fdp
render fdp Nljs    $srcdir/test/appfwk/fdp

render fdc Structs $srcdir/test/appfwk/fdc
render fdc Nljs    $srcdir/test/appfwk/fdc

