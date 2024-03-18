#!/bin/bash

kicad-cli pcb drc \
    -o pcb_drc.rpt \
    --exit-code-violations \
    --severity-all \
    $DRC_INPUT
