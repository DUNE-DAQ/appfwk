#!/usr/bin/env bash

# Really ugly and temporary glue to run moo code generator.
# This will simplify and move into CMake.

mydir=$(dirname $(realpath $BASH_SOURCE))
srcdir=$(dirname $mydir)

# The need for this detail will go away once moo is cleaned up a bit.
oschema=$HOME/dev/moo/examples/oschema
runmoo () {
    moo -g '/lang:ocpp.jsonnet' \
        -J $oschema -T $oschema -J $mydir \
        "$@"
}

# Wrap up the render command.  This bakes in a mapping to file name
# which would be better somehow captured by the schema itself.
render () {
    local name="$1" ; shift
    local What="$1" ; shift
    local outdir="${1:-$srcdir/include/appfwk/${name}}"
    local what="$(echo $What | tr '[:upper:]' '[:lower:]')"
    local tmpl="o${what}.hpp.j2"
    local outhpp="$outdir/${What}.hpp"
    mkdir -p $outdir
    runmoo -A path="dunedaq.appfwk.${name}" \
           -A ctxpath="dunedaq" \
           -A os="appfwk-${name}-schema.jsonnet" \
           render omodel.jsonnet $tmpl \
           > $outhpp
    echo $outhpp
}


render cmd Structs
render cmd Nljs

render fdp Structs
render fdp Nljs

render fdc Structs
render fdc Nljs
