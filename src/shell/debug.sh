#!/bin/bash

for ((i = 0; i < 100; i++)); do
    if [ $i -eq 50 ]; then
        set -x
        echo "Loop var = "$i
        set +x
    fi
done
