# Calculon

Calculon was a senior capstone college project for my computer science class way back in 2017 designed to teach C and lower memory management by writing a mod to asm32 compiler. The base test casses were written by Dr. Clint Staley. It provides a structured environment for running test suites, managing test execution, and validating the outputs which would only function correctly if you wrote your underlying code correctly. 

## Project Structure

```
SourceFinalMulti/
├── workingDirectory/     # Working directory for test execution
│   ├── testingEnvironment/  # Test execution environment
│   ├── allFiles/           # Source files
│   ├── multiFileCalculon   # Multi-file compilation handler
│   └── ClintRecent        # Recent test results
├── InputFiles/          # Test suites and expected outputs
│   ├── Suite1/          # Individual test suites
│   ├── Suite2/
│   ├── Suite3/
│   ├── Suite4/
│   ├── Suite5/
│   ├── Suite6/
│   ├── Suite7/
│   ├── Suite8/
│   ├── Test/            # Additional test cases
│   ├── *.out            # Expected output files for each suite
│   ├── list            # Test suite listing
│   └── MakeOuts        # Makefile outputs
├── shellscript         # Test execution script
└── Calculon           # Main compiler program
```

## Usage

1. **Test Suite Format**:
   - Test suites are defined in `.suite` files
   - Each suite can contain multiple programs and test cases
   - Tests specify input files, expected output, and timeouts

2. **Running Tests**:
   ```bash
   ./shellscript Suite1.suite [d]
   ```
   - Optional 'd' flag enables debug output

3. **Test Configuration**:
   - Input files for test cases
   - Expected output files
   - Timeout settings
   - Command-line arguments

## Error Types

- `DIFF_FAIL`: Output doesn't match expected results
- `RUNTIME_ERR`: Program crashes or fails during execution
- `TIMEOUT`: Program exceeds specified time limit
- `SAFERUN_ERR`: Issues with the SafeRun execution environment

## Implementation Details

- Written in C
- Uses fork/exec for process management
- Implements file I/O redirection
- Supports multiple test programs in a suite
- Provides detailed error reporting and debugging options

## Requirements

- Unix-like operating system
- C compiler (gcc)
- Make (optional, for Makefile support)
- SafeRun utility for timeout management

