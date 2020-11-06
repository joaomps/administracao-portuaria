#!/bin/sh
sort -h $1 > scenario_profs.txt
sort -h $2 > scenario_studs.txt

if diff -q scenario_profs.txt scenario_studs.txt >/dev/null; then
    echo "EQUAL"
else
    echo "There are " $(diff -U 0 scenario_profs.txt scenario_studs.txt | grep ^@ | wc -l) " different lines"
    echo "DIFFERENT"
fi
