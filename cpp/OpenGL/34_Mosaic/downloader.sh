#!/bin/bash

if ! [ -z "$1" ]; then
  count="$1"
else
  count=100
fi

mkdir -p smallImages
cd smallImages
for i in $(eval echo {1..$count}); do
  echo -ne "\r"$i / "$count"
  curl -s -L "https://picsum.photos/400" -o "$i.png";
  convert "$i.png" "$i.bmp";
  rm "$i.png";
done
echo -e "\nDone"
