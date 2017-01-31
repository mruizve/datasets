#!/usr/bin/env bash

# this script is executed automatically by ./evaluation.sh
# for manual execution remember to define the following environment
# variables:

printf "[+] labels histogram... "

# validate variables definitions
if [ -z "$DATASET" ]; then
	echo -e "\r[+] ${RED}labels histogram${WHT}: '\$DATASET' empty or not defined" >&2
	exit 1
fi

if [ -z "$COUNT" ]; then
	echo -e "\r[+] ${RED}labels histogram${WHT}: '\$COUNT' empty or not defined" >&2
	exit 1
fi

if [ -z "$HMIN" -o -z "$HMAX" ]; then
	echo -e "\r[+] ${RED}labels histogram${WHT}: either '\$HMIN' or '\$HMAX' empty or not defined" >&2
	exit 1
fi

# generate histogram values
H="";
for i in $(seq "$HMIN" "$HMAX"); do
	export H=(${H[@]} "$(cat $COUNT|awk -v i=$i '{if($2==i) printf "%d\n",$1}'|wc -l)")
done

# temporally store histogram values on a csv file
echo "0" > "$DATASET/histogram.csv"
for i in ${!H[@]}; do
	printf "%d\n" "${H[$i]}"
done >> "$DATASET/histogram.csv"

# generate the histogram plot
gnuplot<<EOF
   #set terminal postscript eps size 3.2,2.4 enhanced color font 'Helvetica,10' linewidth 1
   set terminal pdfcairo enhanced color dashed font 'Helvetica,6' fontscale 0.5 linewidth 1 rounded size 3.2,1.2
   set output "$DATASET/histogram.pdf"

   # histogram style
   set style data histogram
   set style fill solid border -1
   set boxwidth 0.9

   # key, title and labels
   unset key
   set title "Identities frequency with respect the number of images"
   set xlabel "images"
   set ylabel "identities"

   # axes ranges
   set autoscale
   set xrange [$HMIN:$HMAX]

   # csv file
   set datafile separator ' '

   plot '$DATASET/histogram.csv'
EOF

# delete temporal csv file
rm "$DATASET/histogram.csv"

echo -e "done!\n"
