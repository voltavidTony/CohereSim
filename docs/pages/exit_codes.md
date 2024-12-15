# Exit codes

The exit code provides an indicator as to which exact argument has caused an error. This value may seem redundant as this information is also printed to `stderr`, but it's purpose is to make programmatic interaction with the simulator (scripting) easier.

### Specific values

- `-1` (`255` in Linux): The number of arguments passed does not match any valid argument list
- `0`: The program has executed normally
- `1-8`: The nth argument on the command line is invalid
- Any other value falls under the general format category

### General format

The general format is used when running the program in batch metrics mode, that is, a file containing multiple cache configurations was specified. It is comprised of two bit fields:
- The lower 3 bits deterimine the index of the offending argument
- The remaining bits determine which configuration has caused the error (5 bits on Linux)

The numbering of the arguments, which gets stored in the lower 3 bits, is `1-6` as defined in main.cc, specifically ::args_single_e.

### Limitations

It is worth noting that in Linux, the exit code of a program or shell script is only 8 bits. For instance, if a program returns `-1`, then the value stored in the `$?` variable will be `255`. This means that a maximum of 31 (5 bit count starting at 1) configurations are supported without the potential for overflow in the exit code. For this reason, the exit code is also printed to `stderr` in the format `arg@conf`, where `arg` indicates which argument and `conf` which configuration (0 for command line) caused the error.

On Windows, this issue does not exist as the full 32 bits of the exit code are available to the shell.

### Examples

After issuing `echo $?`, the output was:
- `3` -> Specific value: The 3rd argument on the command line caused an issue
- `42` -> General format `00101_010`: The line size (2nd) argument of the 5th configuration in the configuration file caused an issue
- `62` -> General format `00111_110`: The directory (6th) argument of the 7th configuration in the configuration file caused an issue
