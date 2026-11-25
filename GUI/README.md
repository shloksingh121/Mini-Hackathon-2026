# Student Result Management System — GTK 3 GUI (C)

This application uses your uploaded **updated main.c**, kept as **`updated_main.c`** in this project. Its original data functions are compiled into the GUI and remain unchanged. It is a **native GTK 3 desktop window**, not a website or a Python app.

## macOS — Run

Open Terminal in the extracted project folder and run:

```bash
brew install gtk+3 pkg-config   # only needed if not installed
chmod +x run_mac.sh
./run_mac.sh
```

Or compile manually:

```bash
make clean && make
./student_gui
```

The GTK package is named `gtk+-3.0` for **pkg-config**; in Homebrew it is named `gtk+3`.

If you saved the ZIP in Downloads, you can extract it with Finder, open its new folder, right-click that folder and choose **New Terminal at Folder** (on supported macOS versions), then paste the commands.

## Windows / Linux

Linux with GTK3 development packages installed: `make && ./student_gui`. On Windows use an environment with GTK3 development dependencies (MSYS2).

## Features kept from uploaded code

- Add, edit, delete, search, or inspect up to **100** students
- Full student table with **1–5 numbered subjects** and per-subject backlog labels
- **35/100 minimum in each subject**: each mark below 35 adds one backlog; any backlog -> **F / FAIL**
- Same grade bands: O 85+, A+ 75+, A 65+, B+ 55+, B 45+, C+ 40+, C 35+
- Rank list: all **passing students first**, then failing students, sorted by total within each group; ties share the same rank if their pass/fail group and total match
- Statistics: class pass rate, averages, total backlogs, subject toppers, number failed in each subject
- Load the same ten sample students as your newly uploaded program
- Export report to **`report.csv`**, with name quoting and backlog counts
- Data is read/saved automatically using the original **three-line-per-student `students.txt` format** from the uploaded C file
- If `students.txt` contains invalid records, your original recovery code attempts to save a **`students_backup.txt`** and rewrite valid records. The Mac launcher also creates **`students_before_gui_first_run.txt`** on the first launch if an existing data file is present, to protect that original file

## Data location

The launcher first changes to the project folder. `students.txt`, `report.csv`, and any backup are created **beside `student_gui.c`**. If you previously used the console program, copy its `students.txt` into this directory **before running**, so you keep the existing student data.

Do not open the console and GUI programs to edit the same file at the same time; the latest save would overwrite the other session's changes. Back up important records before upgrading or modifying them.

## Contents

- `updated_main.c`: the original C program you uploaded, included into the GUI build unchanged
- `student_gui.c`: GTK 3 desktop GUI that calls the original C functions
- `Makefile`: C compiler commands
- `run_mac.sh`: build and launch script

### Note on earlier table-style `students.txt` files

Your latest uploaded `updated main.c` uses the **three-line original format**, not the earlier table-style format. The **GUI displays a table**, and `report.csv` contains a tabular export. If you have an old pipe-bordered/table-style `students.txt`, do not overwrite it or assume this version can directly read it. The launcher protects the original with `students_before_gui_first_run.txt`; use `report.csv` or ask for a format migration before converting existing records.
