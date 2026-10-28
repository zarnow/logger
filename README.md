# Asynchronous C++ Logger

A lightweight C++20 logging example. Calls to `log()` and `LOG()` enqueue messages for background workers. `log()` writes to a daily log file and can optionally print to the console; `LOG()` appends C-style formatted messages to `log.txt`, with its console output left commented out. The `log()` timestamp is captured with microsecond precision when the message is submitted.

## Features

- Asynchronous logging through thread-safe queues for `log()` and `LOG()`.
- Daily log rotation, with files named `LOGYYYYMMDD.txt` by default.
- Configurable output directory, file prefix, and date format.
- Optional console output.
- `std::format`-style formatting for log messages.

## Requirements

- A C++20 compiler and standard library with support for `<format>`, time zones (`std::chrono::zoned_time`), and `std::jthread`.
- On Windows, the project includes a VS Code build task for `clang++.exe` in `.vscode/tasks.json`.

## Build and run

Open the project folder in VS Code, open `main.cpp`, then run **C/C++: clang++.exe build active file** from the Build Tasks menu. Run the resulting executable from the project directory so the relative `logs` path points to the project folder.

The sample in `main.cpp` logs a startup message, disables console output, submits 100,000 messages, and reports the elapsed and average time. Because writing happens asynchronously, this timing measures the cost of formatting/enqueuing calls; it does not measure the time needed to finish writing all messages to disk.

## Usage

Include the logger header and call `log()` with a format string and matching arguments:

```cpp
#include "include/logger.hpp"

int main()
{
	log("Application started");
	log("User {} has {} points", "Ada", 42);
}
```

The logger configuration is in the `logger` namespace. Set it before making logging calls:

```cpp
logger::log_to_console = false;       // true by default
logger::log_directory = "logs";       // directory is created automatically
logger::log_file_prefix = "LOG";      // default file: LOGYYYYMMDD.txt
logger::log_file_mask = "{:%Y%m%d}";  // date portion of the filename
```

For example, with the defaults, messages are appended to `logs/LOG20261024.txt` on October 24, 2026. The file's date is based on the local time zone captured with each log event. The current line format displays timestamps to whole seconds, even though the event timestamp is stored at microsecond precision.

## Notes

- The worker thread is started on the first `log()` call. It drains queued messages and is joined during program shutdown.
- Check `stderr` for directory-creation or file-open errors.
- Keep the date mask compatible with the timestamp argument passed to `std::vformat`.
