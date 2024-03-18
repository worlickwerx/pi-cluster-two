#!/bin/bash

# For now generate plots the way PCBWAY wants them:
# https://www.pcbway.com/blog/help_center/How_to_Generate_Gerber_and_Drill_Files_in_KiCad_7_0_ab0d12bb.html
# N.B.  kicad 8.0.1: -o DIR fails with "Output must be a directory"
# so we just plot within the output directory

die() {
    echo $* >&2
    exit 1
}

if test $# -ne 2; then
    echo Usage plot.sh pcbfile zipfile>&2
    exit 1
fi
pcbfile=$(realpath $1)
zipfile=$2

tmpdir=$(mktemp -d) || die "could not create output directory"

pushd $tmpdir
    kicad-cli pcb export gerbers \
        --layers=F.Cu,B.Cu,F.Paste,B.Paste,F.Silkscreen,B.Silkscreen,F.Mask,B.Mask,Edge.Cuts \
        --subtract-soldermask \
        --no-x2 \
        $pcbfile
    test $? -eq 0 || die "could not export gerber files"
    rm -f *.gbrjob

    kicad-cli pcb export drill \
        --format=excellon \
        --excellon-separate-th \
	--excellon-oval-format=route \
	--map-format=ps \
	--drill-origin=absolute \
	--excellon-units=mm \
	--excellon-zeros-format=decimal \
        $pcbfile
    test $? -eq 0 || die "could not export drill files"
popd

echo Generating $zipfile
find $tmpdir -type f -print | zip --quiet --junk-paths $zipfile -@
test $? -eq 0 || die "error creating zip archive"
rm -rf $tmpdir
