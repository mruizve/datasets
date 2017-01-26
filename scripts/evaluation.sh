#!/usr/bin/env bash

# validate input command line arguments
if [ "$#" -ne 1 ]; then
    echo "usage: $0 <path to the dataset>"
    exit 1
fi

# import dataset hierarchy
if [ ! -f $1/info.sh ]; then
	echo "[error] cannot proceed without the dataset info"
	exit 1 
fi

source $1/info.sh

if [ "0" -ne "$?" ]; then
	echo "[error] cannot load dataset info"
	exit 1
fi

export DATASET="$1"

# validate dataset file system hierarchy
variables=("ANNOTATIONS" "INPUT" "OUTPUT")
directories=("$ANNOTATIONS" "$INPUT" "$OUTPUT")
values=("annotations" "input data" "output data")

for i in "${!directories[@]}"; do
	# validate directory definition
	if [ -z "${directories[$i]}" ]; then
		echo "[error] missing definition of the ${values[$i]} folder"
		exit 1
	fi

	# validate directory full path
	if [ ! -d "$DATASET/${directories[$i]}" ]; then
		echo "[error] ${values[$i]} folder does not exists"
		exit 1
	fi

	# export directory with relative path
	export ${variables[$i]}=${directories[$i]}
done

# identify the input data type
if [ -z "$INPUT_EXT" ]; then
	echo "[warning] assuming that the dataset is composed by .png images"
	export INPUT_EXT="png"
fi

# verify annotations (only labels are mandatory)
if [ -f "$DATASET/$ANNOTATIONS/$LABELS" ]; then
	export LABELS="$DATASET/$ANNOTATIONS/$LABELS"
else
	echo "[error] missing data labels"
	exit 1
fi

if [ -f "$DATASET/$ANNOTATIONS/$BBOXES" ]; then
	export BBOXES="$DATASET/$ANNOTATIONS/$BBOXES"
else
	export BBOXES=""
fi

if [ -f "$DATASET/$ANNOTATIONS/$LANDMARKS" ]; then
	export LANDMARKS="$DATASET/$ANNOTATIONS/$LANDMARKS"
else
	export LANDMARKS=""
fi

# export 
export COUNT="$DATASET/count.txt"
export OUTPUT_LABELS="$DATASET/%02d_list.txt"
export OUTPUT_IMAGES="$DATASET/%02d_images.txt"

# get number of images
NIMG=$(cat $LABELS |wc -l)

# get number of labels
NLBL=$(cat $LABELS |cut -d\  -f2|sort|uniq|wc -l)

# generate a record counting the number of images for each label
cat "$LABELS" |cut -d\  -f2|sort|uniq -c|awk '{printf "%d %d\n",$2,$1}' > $COUNT

# get the maximum and minimum number of images for label
HMIN=$(cat $COUNT|awk '{printf "%03d\n",$2}'|sort|uniq|head -n1)
HMIN=${HMIN##*0}
HMAX=$(cat $COUNT|awk '{printf "%03d\n",$2}'|sort|uniq|tail -n1)
HMAX=${HMAX##*0}

# generate histogram plot an retrieve values
H=($(./evaluation_histogram.sh))

# show stats
echo "[+] Dataset statistics";
printf " |-number of images: % 6d\n" "$NIMG";
printf " |-number of labels: % 6d\n" "$NLBL";
echo " |-histogram of labels with respect the number of images:"
# histogram indexes (bins)
printf " | images";
for i in ${!H[@]}; do
	j=$(expr "$i" "+" "1");
	printf "% 5d" "$j";
done
# histogram values
printf "\n | labels";
for i in ${H[@]}; do
	printf "% 5d" "$i";
done
echo;

# get threshold (desired number of images for each label)
REPLY="0"
while [ -z "${REPLY##*[!0-9]*}" ] || [ "$REPLY" -lt "$HMIN" ] || [ "$REPLY" -gt "$HMAX" ]; do
	printf " |< desired image threshold? ";
	read;
done;

# compute thresholded statistics
n="$REPLY"
nIMG=0;
nLBL=0;
for i in ${!H[@]}; do
	j=$(expr "$i" "+" "1");
	if [ "$j" -eq "$n" ]; then
		l=$(expr "${H[$i]}" '*' "$j");
		nIMG=$(expr "$nIMG" "+" "$l");
		nLBL=$(expr "${H[$i]}" "+" "$nLBL");
	fi;
done
# compute labels and images percents
pIMG=$(awk '{printf "%.2f%%\n", 100*$1/$2}' <<< "$nIMG $NIMG");
pLBL=$(awk '{printf "%.2f%%\n", 100*$1/$2}' <<< "$nLBL $NLBL");

# show thresholded statistics
echo " '-number of labels with $n images: $nLBL ($pLBL of all labels) with $nIMG images ($pIMG of all images).";
echo;

# generate output data (file paths)
echo "generating output data:"
OUTPUT_LABELS=$(printf $OUTPUT_LABELS $n);
OUTPUT_IMAGES=$(printf $OUTPUT_IMAGES $n);

# generate output data (list of labels)
printf "  * generating labels list on '$OUTPUT_LABELS'... "
> $OUTPUT_LABELS;
cat $COUNT|awk -v i=$n '{if($2==i) printf "%s\n",$1}' >> $OUTPUT_LABELS;
echo  "done!"

# generate output data (list of labeled images)
printf "  * generating the list of labeled images on '$OUTPUT_IMAGES'... "
> $OUTPUT_IMAGES;
while read -r label || [[ -n "$label" ]]; do
	cat $LABELS|awk  \
	'{
		if($2==l)
		{
			gsub("\n","",$2);
			gsub("\r","",$2);
			gsub(/\..*/,"",$1);
			printf "%s/%s/%s.%s %s\n",o,$2,$1,e,$2;
		}
	}' \
	l="$label" o="$OUTPUT" e="$INPUT_EXT" >> $OUTPUT_IMAGES
done < "$OUTPUT_LABELS"
echo  "done!"

# generate output data (data images)
printf "  * populating the '$DATASET/$OUTPUT' directory... "
./evaluation_data.sh
echo "done!"

# delete extra data
rm $COUNT

exit 0
