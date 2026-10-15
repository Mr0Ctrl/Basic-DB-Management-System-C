# Basic Database Management System in C

A simple file-based database management system implemented in C for a library catalog. It demonstrates two record storage strategies: **fixed-length records** and **variable-length records**.

---

## Features

### 1. Fixed-Length Record System
- **Record Size**: 55 bytes fixed (`Title`: 20B, `Author`: 20B, `ISBN`: 14B, `Deleted`: 1B).
- **Operations**:
  - **Insert**: Reuses deleted slots from the **Avail List** first; appends to the end if none exist.
  - **Print**: Lists all records with their active/deleted status.
  - **Find**: Searches for a book by its ISBN.
  - **Delete**: Soft-deletes a record and adds its offset to the Avail List for reuse.
  - **Compaction**: Automatically purges deleted records on exit into a clean file.

### 2. Variable-Length Record System
- **Record Format**: Stores length indicators before each field to avoid wasting space.
- **Operations**:
  - **Insert**: Appends new records with dynamic string lengths.
  - **Print**: Reads and displays all stored records.
  - **Find**: Searches sequentially for a book by ISBN.

---

## Project Structure

```text
Basic-DB-Management-System-C/
├── fixed-length record system/
│   ├── Fixed-Length-M.c    # Fixed-length implementation
│   ├── input0.txt          # Sample input
│   └── test.dat            # Data file
├── variable-length record system/
│   ├── Variable-Length-M.c # Variable-length implementation
│   ├── input0.txt          # Sample input
│   └── test.dat            # Data file
└── README.md
```

---

## How to Compile & Run

### Fixed-Length System
```bash
cd "fixed-length record system"
gcc Fixed-Length-M.c -o fixed_db
./fixed_db test.dat
```

### Variable-Length System
```bash
cd "variable-length record system"
gcc Variable-Length-M.c -o variable_db
./variable_db test.dat
```

> **Tip**: You can test either program using the sample inputs:
> ```bash
> ./fixed_db test.dat < input0.txt
> ```

---

## Future Improvements

- **Persistent Avail List**: Save deleted slot offsets directly in the file header so free slots persist after closing the program.
- **Variable-Length Deletion**: Support deleting variable-length records with space-reclamation strategies (First-Fit, Best-Fit).
- **Indexing**: Add B-Tree or Hash indexing on ISBN for faster $O(\log N)$ or $O(1)$ lookups instead of linear scans.
- **Multi-Field Search**: Allow searching by author name or book title.
- **Data Validation**: Validate ISBN format/checksums and prevent duplicate entries.
- **Makefile**: Add a unified `Makefile` to build both versions with one command.
