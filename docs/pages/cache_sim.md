# Simulator Operation Manual

[TOC]

The simulator is designed to explore various aspects of a cache configuration. As such, there are multiple modes of operation as described below:

## Metrics Modes

The two metrics modes are where a [trace file](docs/pages/templates.md) is read in and processed by one or more cache configurations. They are designed to generate runtime metrics enabling comparison of cache performance based on the cahce's configuration and the sequence of memory accesses ([trace file](docs/pages/templates.md)) produced by different benchmarks.

The simulator counts the occurrence of many different events and prints them to `stdout` after the last trace is processed. The output is in CSV format, allowing the user to directly import the data into their choice of spreadsheet software (or save it for later) by redirecting `stdout` to a `.csv` file. The output contains one column for each of the statistics defined in the `::bus_msg_e` and `::statistic_e` enums, in addition to a `miss rate` column which is computed using other statistics. Since the simulator processes multicore memory accesses for one or more configurations, `config` and `core` columns are included as identifyers in the output.

The simulator does not process input from `stdin` in these two modes.

### Single

In this mode, a single cache configuration is specified on the command line. Consequently, the value of the `config` field in the output is always 0, which indicates the command line as the source.

The command line arguments are `<cache_size[unit]> <line_size> <associativity> <coherence> <replacer> <directory> <trace_file> [trace_limit]`

- `associativity`: The number of ways in the cache. In other words, the number of cache lines in a set.
- `cache_size`: The size of the cache in bytes. Optionally, 'k' or 'M' can be specified as the `unit`. For instance, one can specify '128k' instead of '131072'.
- `coherence`: The name of the coherence protocol the cache should implement (not case sensitive)
- `directory`: The name of the cirectory protocol the cache should implement (not case sensitive)
- `line_size`: The size of a single cache line in bytes.
- `replacer`: The name of the replacement policy the cache should implement (not case sensitive)
- `trace_file`: The [trace file](docs/pages/templates.md) that the simulator should read in
- `trace_limit`: (Optional) The maximum number of traces the simulator should process

Note:: Every cache dimension must be a power of 2. Any other value is rejected by the simulator.

### Batch

This mode of operation is designed to accelerate the production of the runtime metrics by reading a [trace file](docs/pages/templates.md) once, but processing it across multiple cache configurations in parallel. As such, the value of the `config` field in the output indicates which line of the configs file was the source for the configuration that the accompanying statistics are for. The cache configurations are specified in a configs file, where each line is one instance of the first 6 command line arguments for 'Single Metric' mode, `cache_size[unit]`, `line_size`, `associativity`, `coherence`, `replacer`, and `directory`.

Note:: Due to each configuration processing the trace file in parallel, the order of configurations in the output is arbitrary. A single configuration's output, however, is guaranteed to be contiguous.

The command line arguments for this mode are `<configs_file> <trace_file> [trace_limit]`

- `configs_file`: The file containing a series of cache configurations
- `trace_file`: The [trace file](docs/pages/templates.md) that the simulator should read in
- `trace_limit`: (Optional) The maximum number of traces the simulator should process

## Textbook Mode

Textbook mode is designed not to produce metrics, but instead to allow the user to interactively investigate the behavior of any protocol or policy that a cache may implement.

The simulator processes input received on `stdin` and prints the resulting state and any actions to `stdout` formatted as a table. The table generally reads left-to-right, top-to-bottom, and contains a series of command-action-result entries. The simulator continues processing input until `EOF`, which will never occur if the input is from the terminal. In this case, sending the program interupt signal (`Ctrl+C` or `Ctrl+D`) will close `stdin` instead of terminating the program, thus causing the program to exit naturally.

The single command line argument is the name of the policy/protocol to showcase (not case sensitive). Depending on the choice, textbook mode will respond to a different set of commands (all case insensitive), detailed below:

**Coherence Protocol:**

- `E<n>`: Evict cache line `n`
- `R<n>`: Read access on cache line `n`
- `W<n>`: Write access on cache line `n`
- `X`: Reset all internal state and start a new table

**Replacement Policy:**

- `<tag>`: Access line with a tag of `tag`. Only letters A-Z are allowed as the tag
- `-`: Reset all internal state and start a new table
