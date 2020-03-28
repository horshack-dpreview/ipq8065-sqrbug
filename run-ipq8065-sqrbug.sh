#!/bin/sh
echo Running ipq8065-sqrbug until it fails or 1000 successful iterations
n=1
until [ $n -ge 1000 ]
do
	echo "Test iteration $n"
	./ipq8065-sqrbug
	if [ $? -eq 1 ]; then
		break
	fi
	n=$((n+1))
done
