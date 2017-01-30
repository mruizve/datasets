#!/usr/bin/env bash

# this script is executed automatically by ./evaluation.sh
# for manual execution remember to define the following environment
# variables:

[ -z "$DATASET" ] && echo "(EE) $0: DATASET is not defined" >&2 && exit 1;
[ -z "$INPUT" ] && echo "(EE) $0: INPUT is not defined" >&2 && exit 1;
[ -z "$OUTPUT" ] && echo "(EE) $0: OUTPUT is not defined" >&2 && exit 1;
[ -z "$INPUT_EXT" ] && echo "(EE) $0: INPUT_EXT is not defined" >&2 && exit 1;
[ -z "$OUTPUT_EXT" ] && echo "(EE) $0: OUTPUT_EXT is not defined" >&2 && exit 1;
[ -z "$OUTPUT_SIZE" ] && echo "(EE) $0: OUTPUT_SIZE is not defined" >&2 && exit 1;
[ -z "$BBOXES" ] && echo "(EE) $0: BBOXES is not defined" >&2 && exit 1;
[ -z "$BBOXES_FILTER" ] && echo "(EE) $0: BBOXES_FILTER is not defined" >&2 && exit 1;
[ -z "$LANDMARKS" ] && echo "(EE) $0: LANDMARKS is not defined" >&2 && exit 1;
[ -z "$LANDMARKS_FILTER" ] && echo "(EE) $0: LANDMARKS_FILTER is not defined" >&2 && exit 1;
[ -z "$OUTPUT_LABELS" ] && echo "(EE) $0: OUTPUT_LABELS is not defined" >&2 && exit 1;
[ -z "$OUTPUT_IMAGES" ] && echo "(EE) $0: OUTPUT_IMAGES is not defined" >&2 && exit 1;
[ -z "$OUTPUT_ASSOCIATION" ] && echo "(EE) $0: OUTPUT_ASSOCIATION is not defined" >&2 && exit 1;

# get number of images by identity
n=${OUTPUT_LABELS##*/}
n=${n%_*}

echo "[+] output data"

# generate list of labels
printf " |- generating labels list on '$OUTPUT_LABELS'... "
> "$OUTPUT_LABELS";
cat "$COUNT"|awk -v i=$n '{if($2==i) printf "%s\n",$1}' >> "$OUTPUT_LABELS";
echo  "done!"


printf " |- processing labels... 000000\b\b\b\b\b\b"
> "$OUTPUT_IMAGES"
processed=0
TBEGIN=$(date +%s)
for i in $(cat "$OUTPUT_LABELS"); do
	# extract images with label "$i"
	grep -w "$i" "$LABELS" >> "$OUTPUT_IMAGES"

	# increase labels counter
	((processed+=1))
	printf "%06d\b\b\b\b\b\b" "$processed"
done
sort "$OUTPUT_IMAGES" -o "$OUTPUT_IMAGES"
TEND=$(date +%s)
echo "done in $(($TEND-$TBEGIN)) seconds!"
echo " |- raw list of labeled images written to '$OUTPUT_IMAGES'"

# create data folders
printf " |- creating output directories inside '$DATASET/$OUTPUT'... "
TBEGIN=$(date +%s)
while read -r line; do
	mkdir -p "$DATASET/$OUTPUT/$line"
done < "$OUTPUT_LABELS"
TEND=$(date +%s)
echo "done in $(($TEND-$TBEGIN)) seconds!"

# generate output data
printf " |- processing images... 000000\b\b\b\b\b\b"
> "$OUTPUT_ASSOCIATION"
processed=0
TBEGIN=$(date +%s)
(
	read -r bboxes <&3 && read -r landmarks <&4 # discard first lines
	read -r bboxes <&3 && read -r landmarks <&4 # discard second lines

	# for each labeled image
	while read -r labels; do
		# get corresponding bounding boxes and landmarks
		while [ ! "${bboxes%.*}" = "${labels%.*}" ]; do
			read -r bboxes <&3 && read -r landmarks <&4;
		done

		# convert strings to arrays
		labels=($(echo $labels|tr '\r' ' '))
		bboxes=($(echo $bboxes|tr '\r' ' '))
		landmarks=($(echo $landmarks|tr '\r' ' '))

		# prepare output data
		label="${labels[1]}"
		output="$OUTPUT/${labels[1]}/${labels[0]%.*}.$OUTPUT_EXT"
		input="$INPUT/${labels[0]%.*}.$INPUT_EXT"
		bboxes="$(echo ${bboxes[@]}|awk "$BBOXES_FILTER")"
		landmarks="$(echo "${landmarks[@]}"|awk "$LANDMARKS_FILTER")"

		# copy data association with bounding boxes and landmarks
		echo "$output $label $input $bboxes $landmarks" >> "$OUTPUT_ASSOCIATION";

		# add task to the list
		# echo ${0%/*}/aligner "$input" "$bboxes" "$landmarks" "$OUTPUT_SIZE" "$output" 2>/dev/null

		### create symbolic link inside data folder
		###ln -fs "../../$INPUT/${labels[0]%.*}.$INPUT_EXT" "${output%.*}.$INPUT_EXT"

		# increase image counter
		((processed+=1))
		printf "%06d\b\b\b\b\b\b" "$processed"
	done
) <"$OUTPUT_IMAGES" 3<"$BBOXES" 4<"$LANDMARKS"
TEND=$(date +%s)
echo "done in $(($TEND-$TBEGIN)) seconds!"
echo " |- associated data written to '$OUTPUT_ASSOCIATION'"
