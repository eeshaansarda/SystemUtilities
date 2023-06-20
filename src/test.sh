#!/usr/bin/bash

echo "---Testing for ls---"

for i in "./ls myls.c" "./ls" "./ls ." "./ls ./" "./ls .." "./ls /" "./ls dakjdfh"
do
    echo $i
    $i
    read
done
