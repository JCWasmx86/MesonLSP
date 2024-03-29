#!/usr/bin/env bash
echo "DIFFS = ["
tags=$(git tag | grep -v v1.2.1 | grep -v v2.3.3 | grep -v v2.3.4 | grep -v v2.3.5 | grep -v v2.3.6 | grep -v 2.3.7 | grep -v 2.3.8 | grep -v 2.3.9 | grep -v 2.3.10 | grep -v 2.3.11 | grep -v 2.3.12 | grep -v 2.3.13 | grep -v 2.3.14 | grep -v 2.3.15 | grep -v 2.4.3)
previous_tag=""
for tag in $tags; do
	if [[ $tag == "v1.0" ]]; then
		diff_output=$(git diff --shortstat "$tag")
	else
		diff_output=$(git diff --shortstat "$previous_tag" "$tag")
	fi
	files_changed=$(echo "$diff_output" | awk '{print $1}')
	insertions=$(echo "$diff_output" | awk '{print $4}')
	deletions=$(echo "$diff_output" | awk '{print $6}')
	if [[ $tag == "v1.0" ]]; then
		commit_count=$(git rev-list --count "$tag")
	else
		commit_count=$(git rev-list --count "$previous_tag".."$tag")
	fi
	echo "[$files_changed, $insertions, $deletions, $commit_count],"
	previous_tag=$tag
done
echo "]"

echo "HOURS = ["
git log --format=%aD | sed s/...,.......//g | sed 's/^ //g' | cut -d ' ' -f2 | cut -d ':' -f1 | sort | uniq -c | sed 's/^ *//g' | sed s/\ 0/\ /g | sed s/^/[/g | sed s/\ /,/g | sed s/$/],/g
echo "]"

echo "DAYS = ["
git log --format=%aD | sort | sed s/,.*//g | sort | uniq -c | sort -h | sed s/^\ *//g | sed 's/ /, \"/g' | sed s/$/\"],/g | sed s/^/[/g
echo "]"
