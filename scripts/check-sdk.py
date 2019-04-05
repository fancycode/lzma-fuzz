#!/usr/bin/python -u

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

import os.path
import re
import sys
import urllib2

URL = 'https://www.7-zip.org/sdk.html'

SEARCH_SDK_DOWNLOADS = re.compile(
    r'<TD class="Item" align="center">' +
    r'<A href="a/lzma(\d+)\.7z">Download</A>' +
    r'</TD>', re.I).findall

GET_SDK_VERSION = re.compile(
    r'#define\s+MY_VERSION_NUMBERS\s+"(\d+\.\d+)"').search

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
VERSIONS_FILE = os.path.join(ROOT, 'sdk', 'C', '7zVersion.h')

def normalize_version(s):
  if '.' in s:
    return s

  try:
    version = int(s)
  except ValueError:
    print >> sys.stderr, 'Not a valid version: %s' % (s)
    sys.exit(1)

  return '%.2d.%.2d' % (version / 100, version % 100)

def main():
  with file(VERSIONS_FILE, 'rb') as fp:
    match = GET_SDK_VERSION(fp.read())
    if match is None:
      print >> sys.stderr, 'No version number found in %s' % (VERSIONS_FILE)
      sys.exit(1)

    sdk_version = match.group(1)

  print 'Checking for update of SDK version', sdk_version

  fp = urllib2.urlopen(URL)
  data = fp.read()
  versions = SEARCH_SDK_DOWNLOADS(data)
  if not versions:
    print >> sys.stderr, 'No downloads found, check script.'
    sys.exit(1)

  versions = sorted(map(normalize_version, versions))[::-1]
  print 'Found downloads for versions %s' % (', '.join(versions))
  if sdk_version < versions[0]:
    print >> sys.stderr, 'Found newer version %s' % (versions[0])
    sys.exit(1)

  print 'Already using the latest version'

if __name__ == '__main__':
  main()
