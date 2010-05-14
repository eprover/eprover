#! /usr/local/bin/tcsh -f

do_crossval_silent.awk -iIndexArity    -d0 -tFlat
do_crossval_silent.awk -iIndexSymbol   -d0 -tFlat
do_crossval_silent.awk -iIndexIdentity -d0 -tFlat

do_crossval_silent.awk -iIndexAltTop   -d1 -tFlat
do_crossval_silent.awk -iIndexCSTop    -d1 -tFlat
do_crossval_silent.awk -iIndexESTop    -d1 -tFlat

do_crossval_silent.awk -iIndexTop      -d2 -tFlat
do_crossval_silent.awk -iIndexAltTop   -d2 -tFlat
do_crossval_silent.awk -iIndexCSTop    -d2 -tFlat
do_crossval_silent.awk -iIndexESTop    -d2 -tFlat

do_crossval_silent.awk -iIndexTop      -d3 -tFlat
do_crossval_silent.awk -iIndexAltTop   -d3 -tFlat
do_crossval_silent.awk -iIndexCSTop    -d3 -tFlat
do_crossval_silent.awk -iIndexESTop    -d3 -tFlat

do_crossval_silent.awk -iIndexTop      -d4 -tFlat
do_crossval_silent.awk -iIndexAltTop   -d4 -tFlat
do_crossval_silent.awk -iIndexCSTop    -d4 -tFlat
do_crossval_silent.awk -iIndexESTop    -d4 -tFlat

do_crossval_silent.awk -iIndexTop      -d5 -tFlat
do_crossval_silent.awk -iIndexAltTop   -d5 -tFlat
do_crossval_silent.awk -iIndexCSTop    -d5 -tFlat
do_crossval_silent.awk -iIndexESTop    -d5 -tFlat

do_crossval_silent.awk -iIndexDynamic  -d0 -tFlat



do_crossval_silent.awk -iIndexArity    -d0 -tRecursive
do_crossval_silent.awk -iIndexSymbol   -d0 -tRecursive
do_crossval_silent.awk -iIndexIdentity -d0 -tRecursive

do_crossval_silent.awk -iIndexAltTop   -d1 -tRecursive
do_crossval_silent.awk -iIndexCSTop    -d1 -tRecursive
do_crossval_silent.awk -iIndexESTop    -d1 -tRecursive

do_crossval_silent.awk -iIndexTop      -d2 -tRecursive
do_crossval_silent.awk -iIndexAltTop   -d2 -tRecursive
do_crossval_silent.awk -iIndexCSTop    -d2 -tRecursive
do_crossval_silent.awk -iIndexESTop    -d2 -tRecursive

do_crossval_silent.awk -iIndexTop      -d3 -tRecursive
do_crossval_silent.awk -iIndexAltTop   -d3 -tRecursive
do_crossval_silent.awk -iIndexCSTop    -d3 -tRecursive
do_crossval_silent.awk -iIndexESTop    -d3 -tRecursive

do_crossval_silent.awk -iIndexTop      -d4 -tRecursive
do_crossval_silent.awk -iIndexAltTop   -d4 -tRecursive
do_crossval_silent.awk -iIndexCSTop    -d4 -tRecursive
do_crossval_silent.awk -iIndexESTop    -d4 -tRecursive

do_crossval_silent.awk -iIndexTop      -d5 -tRecursive
do_crossval_silent.awk -iIndexAltTop   -d5 -tRecursive
do_crossval_silent.awk -iIndexCSTop    -d5 -tRecursive
do_crossval_silent.awk -iIndexESTop    -d5 -tRecursive

do_crossval_silent.awk -iIndexDynamic  -d0 -tRecursive



do_crossval_silent.awk -iIndexArity    -d0 -tRecurrent
do_crossval_silent.awk -iIndexSymbol   -d0 -tRecurrent
do_crossval_silent.awk -iIndexIdentity -d0 -tRecurrent

do_crossval_silent.awk -iIndexAltTop   -d1 -tRecurrent
do_crossval_silent.awk -iIndexCSTop    -d1 -tRecurrent
do_crossval_silent.awk -iIndexESTop    -d1 -tRecurrent

do_crossval_silent.awk -iIndexTop      -d2 -tRecurrent
do_crossval_silent.awk -iIndexAltTop   -d2 -tRecurrent
do_crossval_silent.awk -iIndexCSTop    -d2 -tRecurrent
do_crossval_silent.awk -iIndexESTop    -d2 -tRecurrent

do_crossval_silent.awk -iIndexTop      -d3 -tRecurrent
do_crossval_silent.awk -iIndexAltTop   -d3 -tRecurrent
do_crossval_silent.awk -iIndexCSTop    -d3 -tRecurrent
do_crossval_silent.awk -iIndexESTop    -d3 -tRecurrent

do_crossval_silent.awk -iIndexTop      -d4 -tRecurrent
do_crossval_silent.awk -iIndexAltTop   -d4 -tRecurrent
do_crossval_silent.awk -iIndexCSTop    -d4 -tRecurrent
do_crossval_silent.awk -iIndexESTop    -d4 -tRecurrent

do_crossval_silent.awk -iIndexTop      -d5 -tRecurrent
do_crossval_silent.awk -iIndexAltTop   -d5 -tRecurrent
do_crossval_silent.awk -iIndexCSTop    -d5 -tRecurrent
do_crossval_silent.awk -iIndexESTop    -d5 -tRecurrent

do_crossval_silent.awk -iIndexDynamic  -d0 -tRecurrent
