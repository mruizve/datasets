#!/usr/bin/env bash

# example of a dataset script, to be found on the dataset root directory

# file system hierarchy
export ANNOTATIONS="annotations"
export INPUT="images/original"
export OUTPUT="data"

# input data format 
export INPUT_EXT="jpg"
export OUTPUT_EXT="png"

# output data size 
export OUTPUT_SIZE="224x224"

# annotation files
export BBOXES="bboxes.txt"
export LABELS="identities.txt"
export LANDMARKS="landmarks.txt"

# landmarks filtering
# (the face aligner use four facial points: both eyes, the nose and
# the mouth, while this dataset provides two landmarks for the mouth,
# thus this filter leaves unchanged the first tree points while
# computes the mouth midpoint)
export LANDMARKS_FILTER='{
	printf "%s,%s,%s,%s,%s,%s,%s,%s", $2,$3, $4,$5, $6,$7, ($8+$10)/2,($9+$11)/2;
}'
