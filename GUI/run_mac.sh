#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"

if ! command -v pkg-config >/dev/null 2>&1 || ! pkg-config --exists gtk+-3.0; then
  echo 'GTK 3 (gtk+-3.0) and pkg-config are required.'
  echo 'On macOS, install them with: brew install gtk+3 pkg-config'
  exit 1
fi

# Keep a one-time snapshot before the uploaded C loader reads or repairs data.
if [[ -f students.txt && ! -e students_before_gui_first_run.txt ]]; then
  cp -p students.txt students_before_gui_first_run.txt
fi

make
exec ./student_gui
