# Basic Database Management System in C

A lightweight, low-level file-based database management system implemented in C. This project demonstrates core principles of secondary storage file organization and record-level storage management through two primary architectures: **Fixed-Length Records** and **Variable-Length Records**.

The application manages a library catalog storing book records (`Book Title`, `Author Name`, `ISBN`) with support for record insertion, formatted tabular display, primary key searches, soft deletion with free-slot recycling (Avail List), and storage compaction.

---

## Table of Contents

- [Overview](#overview)
- [System Architecture & Storage Design](#system-architecture--storage-design)
  - [1. Fixed-Length Record System](#1-fixed-length-record-system)
  - [2. Variable-Length Record System](#2-variable-length-record-system)
- [Architecture Comparison](#architecture-comparison)
- [Project Structure](#project-structure)
- [Prerequisites](#prerequisites)
- [Compilation & Execution](#compilation--execution)
  - [Fixed-Length System](#fixed-length-system)
  - [Variable-Length System](#variable-length-system)
  - [Automated Testing via Redirection](#automated-testing-via-redirection)
- [Interactive Menus & Usage](#interactive-menus--usage)
- [Future Improvements](#future-improvements)

---

## Overview

Traditional relational databases abstract physical record layouts on disk. This project explores fundamental file organization concepts at the byte and binary I/O level using C standard library system calls (`fread`, `fwrite`, `fseek`, `ftell`).

Key database storage concepts demonstrated:
- **Direct file I/O**: Direct serialization and deserialization of structs to and from binary data files (`.dat`).
- **Tombstone deletion (Soft Delete)**: Marking records as deleted instead of shifting file contents on every delete operation.
- **Avail List (Free Space Management)**: Utilizing an in-memory stack of reclaimed byte offsets to reuse empty slots for incoming insertions, avoiding unnecessary file growth.
- **File Compaction**: Consolidating valid records into a clean data file upon termination to eliminate external fragmentation.
- **Length-indicator field encoding**: Storing variable-length string fields preceded by explicit length headers to eliminate internal fragmentation.

---

## System Architecture & Storage Design

### 1. Fixed-Length Record System

Located in [`fixed-length record system/`](file:///home/mrctrl/Workspace/projects/Basic-DB-Management-System-C/fixed-length%20record%20system):

#### Record Layout (55 Bytes)
Each record has a predetermined, uniform byte size:

```
+----------------------+----------------------+------------------+-------------+
| Book Title (20 bytes)| Author Name (20 bytes)| ISBN (14 bytes)  | Deleted (1b)|
+----------------------+----------------------+------------------+-------------+
|<------------------------------ 55 Bytes ------------------------------------>|
```

- **Book Title**: Fixed array of 20 bytes (`char[20]`)
- **Author Name**: Fixed array of 20 bytes (`char[20]`)
- **ISBN**: Fixed array of 14 bytes (`char[14]`)
- **isDeleted Flag**: 1 byte (`bool`) indicating whether the record is active or deleted (tombstone).

#### Storage Mechanisms:
- **Fast Offset Calculation**: Since every record is precisely 55 bytes, the offset for record $i$ can be directly computed via $Offset = i \times 55$, allowing constant-time random access by slot number.
- **Avail List (Deleted Slot Reclamation)**:
  - When a record is deleted, its `isDeleted` flag is toggled on disk, and its byte offset is pushed onto an in-memory linked-list stack (`AvailListNode`).
  - When inserting a new record, the system first checks the Avail List. If an offset exists, it overwrites the freed slot in-place; otherwise, it appends to `SEEK_END`.
- **Compaction on Exit**:
  - Before exiting, `compactFile()` copies only active records (`!record.isDeleted`) into `temp.dat`, removes the fragmented file, and atomically renames `temp.dat` back to the original database file.

---

### 2. Variable-Length Record System

Located in [`variable-length record system/`](file:///home/mrctrl/Workspace/projects/Basic-DB-Management-System-C/variable-length%20record%20system):

#### Record Layout (Length-Prefixed Fields)
Instead of allocating fixed-width buffers with unused padding bytes (internal fragmentation), fields are dynamically sized and preceded by a 4-byte integer length indicator:

```
+------------+------------------+------------+-------------------+------------+---------------+
| len(Title) | Title Characters | len(Author)| Author Characters | len(ISBN)  | ISBN Char...  |
|  (4 bytes) |   (N bytes)      |  (4 bytes) |     (M bytes)     |  (4 bytes) |   (K bytes)   |
+------------+------------------+------------+-------------------+------------+---------------+
```

#### Storage Mechanisms:
- **Zero Internal Fragmentation**: Strings consume only their exact string lengths on disk.
- **Dynamic Memory Allocation**: Records are allocated dynamically on the heap (`malloc`, `strdup`) upon reading and freed (`freeRecord`) after display or matching, ensuring minimal memory footprint.
- **Sequential Streaming**: Records are appended to the end of the file, and reads sequentially parse lengths and byte streams until EOF.

---

## Architecture Comparison

| Feature | Fixed-Length System | Variable-Length System |
| :--- | :--- | :--- |
| **Record Size** | Static (55 bytes per record) | Dynamic (depends on content length) |
| **Internal Fragmentation** | Yes (unused space in fixed buffers) | None (exact string length stored) |
| **External Fragmentation** | Handled via Avail List & Compaction | None on insert (no deletion implemented) |
| **Record Deletion** | Soft delete + Avail List reuse | Not implemented |
| **Compaction** | Supported (`compactFile` on exit) | Not applicable |
| **Search Method** | Linear scan via `fseek` | Sequential scan with length decoding |
| **Memory Allocation** | Static stack buffers | Dynamic heap allocation (`malloc`/`free`) |

---

## Project Structure

```text
Basic-DB-Management-System-C/
├── fixed-length record system/
│   ├── Fixed-Length-M.c        # Fixed-length DBMS source code
│   ├── input0.txt              # Sample batch input script for testing
│   └── test.dat                # Sample binary database file
├── variable-length record system/
│   ├── Variable-Length-M.c     # Variable-length DBMS source code
│   ├── input0.txt              # Sample batch input script for testing
│   └── test.dat                # Sample binary database file
├── 24-25_COM2553_PA2.pdf       # Project specification document
└── README.md                   # Project documentation
```

---

## Prerequisites

- **C Compiler**: GCC or Clang supporting C99 or later.
- **Operating System**: Linux / macOS / Windows (POSIX compliant or MinGW).

---

## Compilation & Execution

### Fixed-Length System

1. Navigate to the fixed-length directory:
   ```bash
   cd "fixed-length record system"
   ```

2. Compile the source:
   ```bash
   gcc -Wall -Wextra Fixed-Length-M.c -o fixed_db
   ```

3. Run the executable with your desired database file (it will be created if it does not exist):
   ```bash
   ./fixed_db books_fixed.dat
   ```

### Variable-Length System

1. Navigate to the variable-length directory:
   ```bash
   cd "variable-length record system"
   ```

2. Compile the source:
   ```bash
   gcc -Wall -Wextra Variable-Length-M.c -o variable_db
   ```

3. Run the executable with your database file:
   ```bash
   ./variable_db books_variable.dat
   ```

### Automated Testing via Redirection

You can pipe pre-configured commands from `input0.txt` to run automated test insertions and prints:

```bash
# Test fixed-length system
cd "fixed-length record system"
gcc Fixed-Length-M.c -o fixed_db
./fixed_db test.dat < input0.txt

# Test variable-length system
cd "../variable-length record system"
gcc Variable-Length-M.c -o variable_db
./variable_db test.dat < input0.txt
```

---

## Interactive Menus & Usage

### Fixed-Length System Menu
```text
Library Catalog Menu:
1. Insert Record   - Prompt for Title, Author, ISBN; reuses deleted slot or appends
2. Print File      - Displays all records in a formatted table with deleted status
3. Find Record     - Searches for record by ISBN and displays details
4. Delete Record   - Marks record as deleted and enqueues offset into Avail List
0. Exit            - Compacts database file and exits cleanly
```

### Variable-Length System Menu
```text
Library Catalog Menu:
1. Insert Record   - Prompts for variable-length Title, Author, and ISBN; appends to file
2. Print File      - Iterates and parses all records sequentially
3. Find Record     - Searches for a record by ISBN
0. Exit            - Closes file and exits
```

---

## Future Improvements

- [ ] **Persistent Avail List on Disk**: Currently, the Avail List resides in volatile RAM during runtime. Storing the free-list head pointer in a file header block and linking deleted slots directly on disk (embedded free-list chain) would allow slot reuse across program restarts.
- [ ] **Deletion & Space Reclamation for Variable-Length Records**:
  - Implement tombstone markers or record headers for variable-length records.
  - Implement allocation strategies such as **First-Fit**, **Best-Fit**, or **Worst-Fit** along with boundary tags to reclaim variable-sized holes.
  - Add free-space coalescing to merge adjacent freed blocks and prevent external fragmentation.
- [ ] **Indexing (B-Tree / B+ Tree / Hash Index)**:
  - Implement a secondary or primary B+ Tree index on the `ISBN` field to replace $O(N)$ linear scans with $O(\log N)$ point lookups and range queries.
  - Support index persistence with separate `.idx` files.
- [ ] **Secondary Indexes & Multi-Field Search**:
  - Add search capabilities for `Author Name` and `Book Title` (prefix search, partial match, or case-insensitive search).
- [ ] **Data Validation & Integrity Constraints**:
  - Enforce unique ISBN constraints during insertion.
  - Validate ISBN check digits (ISBN-10 and ISBN-13 checksum algorithms).
  - Handle input edge cases, sanitizing non-printable characters and preventing buffer overflows.
- [ ] **Concurrency & Multi-Process Safety**:
  - Implement file locking using POSIX `fcntl` or `flock` to prevent race conditions during concurrent multi-user reads and writes.
  - Implement basic write-ahead logging (WAL) or crash recovery to protect database consistency against unexpected termination during compaction.
- [ ] **Unified Build & CLI Interface**:
  - Add a root-level `Makefile` or `CMakeLists.txt` to compile both systems in a single command.
  - Provide a command-line interface with flag support (e.g., `./db --mode fixed --compact`).
