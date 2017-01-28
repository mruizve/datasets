#!/usr/bin/env bash

# this script is executed automatically by ./evaluation.sh
# for manual execution remember to define the following environment
# variables:

[ -z "$DATASET" ] && echo "(EE) $0: DATASET is not defined" >&2 && exit 1;

# validate dataset file system hierarchy
variables=("ANNOTATIONS" "INPUT" "OUTPUT")
directories=("$ANNOTATIONS" "$INPUT" "$OUTPUT")
values=("annotations" "input data" "output data")

for i in "${!directories[@]}"; do
	# validate directory definition
	if [ -z "${directories[$i]}" ]; then
		echo "(EE) $0: missing definition of the ${values[$i]} folder" >&2
		exit 1
	fi

	# validate directory full path
	if [ ! -d "$DATASET/${directories[$i]}" ]; then
		echo "(EE) $0: ${values[$i]} folder does not exists" >&2
		exit 1
	fi

	# export directory with relative path
	export ${variables[$i]}=${directories[$i]}
done

# identify the input data type
if [ -z "$INPUT_EXT" ]; then
	echo "(WW) $0: assuming that the dataset is composed by .png images" >&2
	export INPUT_EXT="png"
fi

# identify the output data type
if [ -z "$OUTPUT_EXT" ]; then
	echo "(WW) $0: assuming that the dataset is composed by .png images" >&2
	export OUTPUT_EXT="png"
fi

# identify the output dimensions
if [ -z "$OUTPUT_SIZE" ]; then
	echo "(WW) $0: assuming that the output images size is 224x224" >&2
	export OUTPUT_SIZE="224x224"
fi

# verify annotations:
# -- labels (mandatory)
if [ -f "$DATASET/$ANNOTATIONS/$LABELS" ]; then
	export LABELS="$DATASET/$ANNOTATIONS/$LABELS"
else
	echo "(EE) $0: missing data labels" >&2
	exit 1
fi

# -- faces bounding boxes (optional)
if [ -f "$DATASET/$ANNOTATIONS/$BBOXES" ]; then
	export BBOXES="$DATASET/$ANNOTATIONS/$BBOXES"
else
	export BBOXES=""
fi

# -- facial landmarks (mandatory)
if [ -f "$DATASET/$ANNOTATIONS/$LANDMARKS" ]; then
	export LANDMARKS="$DATASET/$ANNOTATIONS/$LANDMARKS"
else
	echo "(EE) $0: missing facial landmarks" >&2
	exit 1
fi

# -- landmarks filter (optional)
if [ -z "$LANDMARKS_FILTER" ]; then
	export LANDMARKS_FILTER='{ printf "%s",$0; }'
fi

# I/O file streams
export COUNT="$DATASET/count.txt"
export OUTPUT_LABELS="$DATASET/%02d_list.txt"
export OUTPUT_IMAGES="$DATASET/%02d_images.txt"
export OUTPUT_BBOXES="$DATASET/%02d_bboxes.txt"
export OUTPUT_LANDMARKS="$DATASET/%02d_landmarks.txt"

