#!/usr/bin/env bash

# this script is executed automatically by ./evaluation.sh
# for manual execution remember to define the following environment
# variables:

echo "[+] output data"

# validate variables definitions
printf " |- check variables... "
if [ -z "$DATASET" ]; then
	echo -e "\r[+] ${RED}check variables${WHT}: '\$DATASET' empty or not defined" >&2
	exit 1
fi

if [ -z "$INPUT" -o -z "$OUTPUT" ]; then
	echo -e "\r[+] ${RED}check variables${WHT}: either '\$INPUT' or '\$OUTPUT' empty or not defined" >&2
	exit 1
fi

if [ -z "$INPUT_EXT" -o -z "$OUTPUT_EXT" -o -z "$OUTPUT_SIZE" ]; then
	echo -e "\r[+] ${RED}check variables${WHT}: missing some input/output data format information" >&2
	exit 1
fi

if [ -z "$LABELS" -o -z "$BBOXES" -o -z "$LANDMARKS" -o -z "$COUNT" ]; then
	echo -e "\r[+] ${RED}check variables${WHT}: missing some annotations files" >&2
	exit 1
fi

if [ -z "$BBOXES_FILTER" -o -z "$LANDMARKS_FILTER" ]; then
	echo -e "\r[+] ${RED}check variables${WHT}: missing some annotations filters" >&2
	exit 1
fi

if [ -z "$OUTPUT_LABELS" -o -z "$OUTPUT_IMAGES" -o -z "$OUTPUT_ASSOCIATION" ]; then
	echo -e "\r[+] ${RED}check variables${WHT}: missing some output files paths" >&2
	exit 1
fi
echo "done!"

# get number of images by identity
n=${OUTPUT_LABELS##*/}
n=${n%_*}

# generate list of labels
printf " |- generating labels list... "
> "$OUTPUT_LABELS";
cat "$COUNT"|awk -v i="$n" '{if($2==i) printf "%s\n",$1}' >> "$OUTPUT_LABELS";
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

# create data folders
printf " |- creating output directories inside '$DATASET/$OUTPUT'... "
TBEGIN=$(date +%s)
while read -r line; do
	mkdir -p "$DATASET/$OUTPUT/$line"
done < "$OUTPUT_LABELS"
TEND=$(date +%s)
echo "done in $(($TEND-$TBEGIN)) seconds!"

# generate data association
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

# generate output data
printf " '- generating training/testing data ... 000000\b\b\b\b\b\b"
TBEGIN=$(date +%s)
${0%/*}/aligner "$DATASET" "${OUTPUT_ASSOCIATION##*/}" "$OUTPUT_SIZE" || exit 1
TEND=$(date +%s)
echo "done in $(($TEND-$TBEGIN)) seconds!"
