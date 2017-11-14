#!/bin/bash
php map_test.php $1 | grep -v -e "^\+" | grep -v -e "^#" | grep -v -e "^'" | grep -v -e "^\."
