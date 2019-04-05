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

function dump_file {
	echo "Dumping $1 (readable) ..."
	/usr/bin/hd $1
	echo
	echo "Dumping $1 (raw) ..."
	/usr/bin/hexdump -v -e '32/1 "%02X" 1 "\n"' $1 ; echo
}

CRASH_FILES=$(ls crash-* 2>/dev/null || true)
for filename in ${CRASH_FILES}; do
	dump_file "${filename}"
done

LEAK_FILES=$(ls leak-* 2>/dev/null || true)
for filename in ${LEAK_FILES}; do
	dump_file "${filename}"
done
