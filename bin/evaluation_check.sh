#!/usr/bin/env bash

# this script is executed automatically by ./evaluation.sh
# for manual execution remember to define the following environment
# variables:

[ -z "$DATASET" ] && echo "(EE) $0: DATASET is not defined" >&2 && exit 1;

echo "[+] dataset's 'info.sh' check"

# validate dataset file system hierarchy
variables=("ANNOTATIONS" "INPUT" "OUTPUT")
directories=("$ANNOTATIONS" "$INPUT" "$OUTPUT")
values=("annotations" "input data" "output data")

printf " |- file system hierarchy... "
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
echo "done!"

# identify the input and output data formats
printf " |- data formats... "
if [ -z "$INPUT_EXT" ]; then
	echo "(WW) $0: assuming that the dataset is composed by .png images" >&2
	export INPUT_EXT="png"
fi

if [ -z "$OUTPUT_EXT" ]; then
	echo "(WW) $0: assuming that the dataset is composed by .png images" >&2
	export OUTPUT_EXT="png"
fi

if [ -z "$OUTPUT_SIZE" ]; then
	echo "(WW) $0: assuming that the output images size is 224x224" >&2
	export OUTPUT_SIZE="224x224"
fi
echo "input={$INPUT_EXT,?x?}, output={$OUTPUT_EXT,$OUTPUT_SIZE}."

# verify annotations:
printf " '- annotations... "

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
	echo "(EE) $0: missing faces bounding boxes" >&2
	exit 1
fi

# -- bounding boxes filter (optional)
if [ -z "$BBOXES_FILTER" ]; then
	export BBOXES_FILTER='{ printf "%s",$0; }'
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

echo "done!"
echo

# I/O file streams
export COUNT="$DATASET/count.txt"
export OUTPUT_LABELS="$DATASET/%02d_list.txt"
export OUTPUT_IMAGES="$DATASET/%02d_images.txt"
export OUTPUT_ASSOCIATION="$DATASET/%02d_association.txt"

