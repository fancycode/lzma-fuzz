#!/bin/bash -eu

##
## @copyright Copyright (c) 2019 Joachim Bauch <mail@joachim-bauch.de>
##
## @license GNU GPL version 3 or any later version
##
## This program is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program.  If not, see <https://www.gnu.org/licenses/>.
##

if [ "$FUZZER" = "check-sdk" ]; then
	exec ./scripts/check-sdk.py
fi

echo "Building $FUZZER ..."
make

if [ -d "corpus/$FUZZER" ]; then
	echo "Running $FUZZER against corpus ..."
	./$FUZZER corpus/$FUZZER/*
fi

echo "Running $FUZZER ..."
./$FUZZER -max_total_time=10
