#!/usr/bin/env bash

# this script is executed automatically by ./evaluation.sh
# for manual execution remember to define the following environment
# variables: $OUTPUT_LABELS and $OUTPUT_IMAGES

[ -z "$OUTPUT_LABELS" ] && echo "OUTPUT_LABELS is not defined" && exit 1;
[ -z "$OUTPUT_IMAGES" ] && echo "OUTPUT_IMAGES is not defined" && exit 1;

# get number of images by identity
n=${OUTPUT_LABELS##*/}
n=${n%_*}

# generate output data (create identities data folders)
while read -r line || [[ -n "$line" ]]; do
	mkdir -p "$DATASET/$OUTPUT/$line"
done < "$OUTPUT_LABELS"

# generate output data (create symbolic links to images)
while read -r line || [[ -n "$line" ]]; do
	# tokenize line
	line=($line)

	# get identity number and the relative image path
	id="${line[1]}"
	image="${line[0]}"

	# create symbolic link inside data folder
	ln -fs "../../$INPUT/${image##*/}" "$DATASET/$image"
done < "$OUTPUT_IMAGES"

exit 0
