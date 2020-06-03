# program-flow-analysis
Basic static code parsing/ analysis for C programs.

NOTE: The parser covers a small subset of the C language.

## Dependencies
The following dependencies are required: `flex` `bison`.

For Debian based distros, run: `sudo apt install -y flex bison`

## Building
```
mkdir build && cd build
cmake ..
make
```

## Usage
`analyzer <source_file>`
