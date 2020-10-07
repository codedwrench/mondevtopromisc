# MIT License
#
# Copyright (c) 2020 Aiden Woodruff
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

find_path(PDCURSES_INCLUDE_DIR curses.h)
find_library(PDCURSES_LIBRARY pdcurses)

file(STRINGS "${PDCURSES_INCLUDE_DIR}/curses.h" _PDCURSES_VERSION_STR REGEX "^#define PDC_VERDOT[ \t]+\\\".*\\\"$")

string(REGEX REPLACE "^#define PDC_VERDOT[ \t]+\\\"\\(.*\\)\\\"$" "\\1" PDCURSES_VERSION "${_PDCURSES_VERSION_STR}")

set(PDCURSES_LIBRARIES ${PDCURSES_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
PDCurses
REQUIRED_VARS
  PDCURSES_LIBRARY
  PDCURSES_INCLUDE_DIR
VERSION_VAR PDCURSES_VERSION)

set(HAVE_CURSES_H TRUE)

mark_as_advanced(PDCURSES_LIBRARY PDCURSES_LIBRARIES PDCURSES_INCLUDE_DIR PDCURSES_VERSION)