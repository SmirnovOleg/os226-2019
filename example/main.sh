#!/bin/bash

while read student_id; do
    let logbook[student_id]+=1;
done

most_common_id=0
most_common_times=0

for student_id in ${!logbook[@]}; do
    if (( ${logbook[student_id]} > $most_common_times )); then
        most_common_times=${logbook[student_id]}
        most_common_id=$student_id
    fi
done

echo $most_common_id