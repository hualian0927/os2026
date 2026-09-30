#!/bin/sh
# Local lab1 self-check, not the course's official grading script.
set -eu
cd "$(dirname "$0")/.."
echo 'Lab1 local self-check (not an official course score)'
exec make --no-print-directory check
