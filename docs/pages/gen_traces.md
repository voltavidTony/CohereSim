# Trace file generation

[TOC]

## Notes

1. The trace format is described on the [main page](index.html) under Tool Suite > Trace file generation
2. `libs` and `tools` packages are not runnable. Probably since they are libraries or tools.
3. The group `netapps` work initially, but after a few seconds subsequent runs of `netapps` inexplicably hang indefinitely. There are three benchmarks under that category, and each of them individually hang indefinitely. After a system restart they work again.

## Setup instructions

### 1 Install Ubuntu on WSL2 (skip if your computer is running Linux)

1. Turn on 'Windows Subsystem for Linux' feature
2. Install Ubuntu with `wsl --install -d Ubuntu` (make sure the framework is WSL2)
3. Lauch Ubuntu to complete installation and setup user account

### 2 Initialize local copy

If you haven't already, download this repo with the following command: `git clone --recurse-submodules https://github.com/voltavidTony/cache-sim.git`. If you already have this repo, but not the gem5 and parsec-benchmark sub-modules, then run this command: `git submodule update --init`

### 3 Build gem5

From within the `gem5` directory:

1. Install necessary dependencies. Depending on the Ubuntu version, run (I had to also install clang to get the `png` and `hdf5` libraries to work):<br>`sudo apt install build-essential scons python3-dev git pre-commit zlib1g zlib1g-dev libprotobuf-dev protobuf-compiler libprotoc-dev libgoogle-perftools-dev libboost-all-dev  libhdf5-serial-dev python3-pydot python3-venv python3-tk mypy m4 libcapstone-dev libpng-dev libelf-dev pkg-config wget cmake doxygen`
2. Make necessary changes to `gem5/src/mem/abstract_mem.cc`. At the time of writing, the version of gem5 used is v24.0.0.1, but since the changes are based on the existing `Memoryaccess` debug flag, they will likely work as-is for quite a while:
   1. Before the `AbstractMemory::access` method on line 380, insert the following code snippet:
   ```
   #define ECE506TRACE(A) std::string reqName = system()->getRequestorName( \
   pkt->req->requestorId()); ::gem5::trace::getDebugLogger()->dprintf_flag( \
   (::gem5::Tick)-1, std::string(), "ECE506Trace", "%s\t%c\t%x\n", \
   reqName.substr(15, reqName.length() - 25), A, pkt->getAddr());
   ```
   2. Then, after the `TRACE_PACKET` macro on line 455 (451 of the original file), insert the following:
   ```
   ECE506TRACE(pkt->req->isInstFetch() ? 'i' : 'r');
   ```
   3. Finally, after the `TRACE_PACKET` macro on line 478 (473 of the original file), insert the following:
   ```
   ECE506TRACE('w');
   ```
3. Build the project with SCons (will take several minutes): `scons build/{ISA}/gem5.{variant} -j {cpus}`
   - {ISA}: target (guest) instruction set architecture (ALL for all possible ISAs). Gem5 should be built to support the same ISA as your local system, since the PARSEC benchmarks will be compiled to run natively
   - {variant}: compilation settings (`fast` is recommended to increase simulation speed, and `opt` for debugging error messages during simulation. `debug` is for debugging the simulator itself and shouldn't be needed). Note: The trace generation script uses the `fast` variant
   - {cpus}: specify # of threads with optional `-j` argument (strongly recommended for initial build, minor impact on incremental builds)
   - [Explanation of arguments here](https://www.gem5.org/documentation/general_docs/building#building-with-scons)

### 4 Build PARSEC benchmarks

From within the `parsec-benchmark` directory:

1. Install necessary dependencies (These were the extra dependencies I needed. Your mileage may vary):
   - parsec.glib: `sudo apt install gettext texinfo`
   - parsec.mesa: `sudo apt install libx11-dev libxext-dev libxt-dev libxmu-dev libxi-dev`
2. Download necessary input files: `./get-inputs`
3. Build PARSEC (will take several minutes):
   - `./bin/parsecmgmt -a build -p all {nthreads}` (not sure if the script uses the threadcount argument when building)
   - If a command is not found, try to run the command and Ubuntu will tell you what package to install
   - If a certain package is missing, chances are the correct package to install is `lib{package}-dev`
4. Test if the benchmark suite functions with the (default) test inputs: `./bin/parsecmgmt -a run -p all`

## Run the trace generation script

Note:: This script uses the `fast` variant of gem5. It is located in the `tools` directory, but can be run from anywhere.

Usage: `./gen_trace.sh [benchmark] {inputsize} {nthreads} {ncpus}`
- `benchmark`: Specify the benchmark to run
  - Can be of format `<package>.<benchmark>` or just `<benchmark>`
- `inputsize`: Determines the size of the input
  - Can be one of: `test`, `simdev`, `simsmall`, `simmedium`, `simlarge`, `native`
  - Defaults to: test
- `nthreads`: Determines the number of worker threads to spawn
  - Exact implementation can vary between benchmarks
  - Defaults to: 8
- `ncpus`: Determines the number of CPUs in the gem5 simulation
  - Defaults to: 16
