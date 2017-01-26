#!/usr/bin/env bash

# this script is executed automatically by ./evaluation.sh
# for manual execution remember to define the following environment
# variables: $COUNT

[ -z "$COUNT" ] && echo "COUNT is not defined" && exit 1;

# get the maximum and minimum number of images for identity
HMIN=$(cat $COUNT|awk '{printf "%03d\n",$2}'|sort|uniq|head -n1)
HMIN=${HMIN##*0}
HMAX=$(cat $COUNT|awk '{printf "%03d\n",$2}'|sort|uniq|tail -n1)
HMAX=${HMAX##*0}

# generate histogram values
H="";
for i in $(seq "$HMIN" "$HMAX"); do
	H=(${H[@]} "$(cat $COUNT|awk -v i=$i '{if($2==i) printf "%d\n",$1}'|wc -l)")
done
echo ${H[@]}

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

exit 0
