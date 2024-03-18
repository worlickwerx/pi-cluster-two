#!/bin/bash

kicad-cli sch erc \
    -o sch_erc.rpt \
    --exit-code-violations \
    --severity-all \
    $ERC_INPUT
