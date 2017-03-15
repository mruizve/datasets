#!/usr/bin/env bash

# this script is executed automatically by ./evaluation.sh
# for manual execution remember to define the following environment
# variables:

echo "[+] dataset's check"

# validate the given dataset's path
printf " |- dataset's path... "
if [ -z "$DATASET" ]; then
	echo -e "\r '- ${RED}dataset path${WHT}: '\$DATASET' empty or not defined" >&2
	exit 1;
fi

if [ ! -d "$DATASET" ]; then
	echo -e "\r '- ${RED}dataset path${WHT}: '$DATASET' not a valid directory" >&2
	exit 1
fi
echo "done!"

# import dataset hierarchy
printf " |- loading 'info.sh'... "
if [ ! -f $1/info.sh ]; then
	echo -e "\r '- ${RED}loading 'info.sh'${WHT}: '$DATASET/info.sh' not a valid file" >&2
	exit 1 
else
	source $1/info.sh
	if [ "0" -ne "$?" ]; then
		echo -e "\r '- ${RED}loading 'info.sh'${WHT}: unexpected error from '$DATASET/info.sh'" >&2
		exit 1
	fi
fi
echo "done"!

# validate dataset file system hierarchy
variables=("ANNOTATIONS" "INPUT" "OUTPUT")
directories=("$ANNOTATIONS" "$INPUT" "$OUTPUT")
values=("annotations" "input data" "output data")

printf " |- file system hierarchy... "
for i in "${!directories[@]}"; do
	# validate directory definition
	if [ -z "${directories[$i]}" ]; then
		echo -e "\r '- ${RED}file system hierarchy${WHT}: missing definition of '\$${variables[$i]}'" >&2
		exit 1
	fi

	# validate directory full path
	if [ ! -d "$DATASET/${directories[$i]}" ]; then
		echo -e "\r '- ${RED}file system hierarchy${WHT}: '\$${variables[$i]}' value is not a valid directory" >&2
		exit 1
	fi

	# export directory with relative path
	export ${variables[$i]}=${directories[$i]}
done
echo "done!"

# identify the input and output data formats
printf " |- data formats... "
if [ -z "$INPUT_EXT" ]; then
	export INPUT_EXT="png"
	printf "input={${YLL}png${WHT},?x?}, "
else
	printf "input={$INPUT_EXT,?x?}, "
fi

if [ -z "$OUTPUT_EXT" ]; then
	export OUTPUT_EXT="png"
	printf "output={${YLL}png${WHT},"
else
	printf "output={$OUTPUT_EXT,"
fi

if [ -z "$OUTPUT_SIZE" ]; then
	export OUTPUT_SIZE="224x224"
	echo -e "${YLL}224x224${WHT}}$"
else
	echo "$OUTPUT_SIZE}"
fi

# verify annotations:
variables=("LABELS" "BBOXES" "LANDMARKS" "ATTRIBUTES")
files=("$LABELS" "$BBOXES" "$LANDMARKS" "$ATTRIBUTES")
values=("labels" "bounding boxes" "facial landmarks" "face attributes")

printf " |- annotations... "
for i in "${!files[@]}"; do
	# validate annotation definition
	if [ -z "${files[$i]}" ]; then
		echo -e "\r '- ${RED}annotations${WHT}: missing definition of '\$${variables[$i]}'" >&2
		exit 1
	fi

	# validate file path
	if [ ! -f "$DATASET/$ANNOTATIONS/${files[$i]}" ]; then
		echo -e "\r '- ${RED}annotations${WHT}: '\$${variables[$i]}' value is not a valid file" >&2
		exit 1
	fi

	# export annotations files with absolute path
	export ${variables[$i]}="$DATASET/$ANNOTATIONS/${files[$i]}"
done
echo "done!"

# bounding boxes, landmarks filters and attributes (optional)
printf " '- annotation filters... "
if [ -z "$BBOXES_FILTER" ]; then
	export BBOXES_FILTER='{ printf "%s",$0; }'
	filters="default ${YLL}bboxes${WHT},"
else
	filters="custom bounding boxes,"
fi

if [ -z "$LANDMARKS_FILTER" ]; then
	export LANDMARKS_FILTER='{ printf "%s",$0; }'
	filters="$filters default ${YLL}facial landmarks${WHT} and"
else
	filters="$filters custom facial landmarks and"
fi

if [ -z "$ATTRIBUTES_FILTER" ]; then
	export ATTRIBUTES_FILTER='{ printf "%s",$0; }'
	filters="$filters default ${YLL}facial attributes${WHT}"
else
	filters="$filters custom facial attributes"
fi

echo -e "using $filters filters\n"

# I/O file streams
export COUNT="$DATASET/count.txt"
export OUTPUT_LABELS="$DATASET/%02d_list.txt"
export OUTPUT_IMAGES="$DATASET/%02d_images.txt"
export OUTPUT_ASSOCIATION="$DATASET/%02d_association.txt"
