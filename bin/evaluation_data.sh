#!/usr/bin/env bash

# this script is executed automatically by ./evaluation.sh
# for manual execution remember to define the following environment
# variables:

[ -z "$DATASET" ] && echo "(EE) $0: DATASET is not defined" >&2 && exit 1;
[ -z "$INPUT" ] && echo "(EE) $0: INPUT is not defined" >&2 && exit 1;
[ -z "$OUTPUT" ] && echo "(EE) $0: OUTPUT is not defined" >&2 && exit 1;
[ -z "$OUTPUT_EXT" ] && echo "(EE) $0: OUTPUT_EXT is not defined" >&2 && exit 1;
[ -z "$OUTPUT_SIZE" ] && echo "(EE) $0: OUTPUT_SIZE is not defined" >&2 && exit 1;
[ -z "$BBOXES" ] && echo "(EE) $0: BBOXES is not defined" >&2 && exit 1;
[ -z "$LANDMARKS" ] && echo "(EE) $0: LANDMARKS is not defined" >&2 && exit 1;
[ -z "$LANDMARKS_FILTER" ] && echo "(EE) $0: LANDMARKS_FILTER is not defined" >&2 && exit 1;
[ -z "$OUTPUT_LABELS" ] && echo "(EE) $0: OUTPUT_LABELS is not defined" >&2 && exit 1;
[ -z "$OUTPUT_IMAGES" ] && echo "(EE) $0: OUTPUT_IMAGES is not defined" >&2 && exit 1;

# get number of images by identity
n=${OUTPUT_LABELS##*/}
n=${n%_*}

echo "[+] output data"

# generate list of labels
printf " |- generating labels list on '$OUTPUT_LABELS'... "
> $OUTPUT_LABELS;
cat $COUNT|awk -v i=$n '{if($2==i) printf "%s\n",$1}' >> $OUTPUT_LABELS;
echo  "done!"

# create data folders)
printf " |- generating output directories inside '$DATASET/$OUTPUT'... "
while read -r line; do
	mkdir -p "$DATASET/$OUTPUT/$line"
done < "$OUTPUT_LABELS"
echo "done!"

# generate output data
printf " |- procesing images... 000000\b\b\b\b\b\b"
> $OUTPUT_IMAGES;
> $OUTPUT_BBOXES;
> $OUTPUT_LANDMARKS;
processed=0;
(
	read -r bboxes <&4 && read -r landmarks <&5 # discard first lines
	read -r bboxes <&4 && read -r landmarks <&5 # discard second lines
	while read -r labels <&3 && read -r bboxes <&4 && read -r landmarks <&5; do
		# convert strings to arrays
		labels=($(echo $labels|tr '\r' ' '))
		bboxes=($(echo $bboxes|tr '\r' ' '))
		landmarks=($(echo $landmarks|tr '\r' ' '))

		# if the image label belongs to the list, then
		if [ -n "$(grep -m 1 "\b${labels[1]}\b" "$OUTPUT_LABELS")" ]; then
			# add image to the labeled list
			echo "$OUTPUT/${labels[1]}/${labels[0]%.*}.$OUTPUT_EXT ${labels[1]}" >> "$OUTPUT_IMAGES";

			# copy bounding boxes and landmarks
			echo "${labels[0]} ${bboxes[@]:1}" >> "$OUTPUT_BBOXES";
			echo "${labels[0]} ${landmarks[@]:1}" >> "$OUTPUT_LANDMARKS";

			# prepare aligner command line arguments
			#input="$DATASET/$INPUT/${labels[0]%.*}.$INPUT_EXT"
			#landmarks="$(echo "${landmarks[@]}"|awk "$LANDMARKS_FILTER")"
			#output="$DATASET/$OUTPUT/${labels[1]}/${labels[0]%.*}.$OUTPUT_EXT"

			# add task to the list
			#${0%/*}/aligner "$input" "$landmarks" "$OUTPUT_SIZE" "$output" 2>/dev/null

			### create symbolic link inside data folder
			###ln -fs "../../$INPUT/${labels[0]%.*}.$INPUT_EXT" "${output%.*}.$INPUT_EXT"

			# increase image counter
			((processed+=1))
			printf "%06d\b\b\b\b\b\b" $processed
		fi
	done
) 3<"$LABELS" 4<"$BBOXES" 5<"$LANDMARKS"
echo "done! "
echo " |- labeled images generated on '$OUTPUT_IMAGES'"
echo " |- bounding boxes copied to '$OUTPUT_BBOXES'"
echo " '- facial landmarks copied to '$OUTPUT_LANDMARKS'"
