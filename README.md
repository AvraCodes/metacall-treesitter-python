# Tree-sitter Python Static Parser

Static analysis tool that extracts public functions, classes, and methods from Python files using Tree-sitter.

Part of Google Summer of Code 2026 project with MetaCall.

## Features

- Parses Python files using Tree-sitter
- Extracts public functions (filters out `_private` and `__dunder__`)
- Extracts public classes with public methods
- Outputs JSON similar to MetaCall inspect schema
- Static analysis only - no runtime required

## Build

```bash
# Clone dependencies
git clone https://github.com/tree-sitter/tree-sitter.git
git clone https://github.com/tree-sitter/tree-sitter-python.git

# Build
mkdir build && cd build
cmake ..
make
```

## Run

```bash
./parser_test
```

Or save output to file:

```bash
./parser_test > output.json
```

## Project Structure

```
.
├── src/
│   └── main.cpp           # Main parser implementation
├── CMakeLists.txt         # Build configuration
├── test.py                # Example Python file
├── tree-sitter/           # Tree-sitter core library (git clone)
└── tree-sitter-python/    # Python grammar (git clone)
```

## Output Format

```json
{
  "name": "test.py",
  "scope": {
    "funcs": [
      {
        "name": "greet",
        "signature": {
          "ret": { "type": { "name": "unknown" } },
          "args": [
            { "name": "name", "type": { "name": "unknown" } }
          ]
        },
        "async": false
      }
    ],
    "classes": [
      {
        "name": "Person",
        "methods": [
          { "name": "say_hello", "async": false }
        ]
      }
    ]
  }
}
```

## How It Works

1. **Parse:** Tree-sitter parses Python source into an Abstract Syntax Tree (AST)
2. **Walk:** Recursively traverse the AST looking for function and class definitions
3. **Extract:** Use Tree-sitter field names to get function/class names and parameters
4. **Filter:** Skip private (`_name`) and dunder (`__name__`) definitions
5. **Output:** Generate JSON matching MetaCall's inspect schema format

## Next Steps

- Integrate with MetaCall's reflect and serialization API
- Add support for async functions
- Extend to other languages (JavaScript, TypeScript, etc.)
