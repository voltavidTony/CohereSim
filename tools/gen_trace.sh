#!/bin/bash

# Required arguments
if [ $# -eq 0 ]; then
    echo "Usage: ./$(basename $0) [benchmark] {inputsize} {nthreads} {ncpus}"
    echo '  - benchmark: Specify the benchmark to run'
    echo '    - Can be of format "<package>.<benchmark>" or just "<benchmark>"'
    echo '  - inputsize: Determines the size of the input'
    echo '    - Can be one of: test, simdev, simsmall, simmedium, simlarge, native'
    echo '    - Defaults to: test'
    echo '  - nthreads: Determines the number of worker threads to spawn'
    echo '    - Exact implementation can vary between benchmarks'
    echo '    - Defaults to: 8'
    echo '  - ncpus: Determines the number of CPUs in the gem5 simulation'
    echo '    - Defaults to: 16'
    exit 0
fi

# Default parameter
INPUTSIZE=${2:-test}
NTHREADS=${3:-8}
NCPUS=${4:-16}

# Local directories
cd "$(dirname "$0")"
TOOLSDIR="$(pwd)"
GEMDIR="$(pwd)/gem5"
PARSECDIR="$(pwd)/parsec-benchmark"

# Determine platform
source get_platform.sh

# Compile the trace extractor program
gcc -DNCPU=$NCPUS extractor.c -o extractor || {
    echo "Please ensure the trace extractor program is present!"
    exit 1
}

# Non-local directories
cd ..
OUTDIR="$(pwd)/traces"

# Validate package argument
if [[ ! $1 =~ ^((parsec|splash2x)\.)?([a-z_0-9]+)$ ]]; then
    echo "Malformed benchmark argument! ({[parsec|splash2x].}<benchmark>)"
    exit 1
fi
BENCHMARK=${BASH_REMATCH[3]}

# Try to find package
if [ -f "$PARSECDIR/config/packages/$1.pkgconf" ]; then
    source "$PARSECDIR/config/packages/$1.pkgconf"
    SUITE=${BASH_REMATCH[2]}
elif [ -z ${BASH_REMATCH[1]} ]; then
    if [ -f "$PARSECDIR/config/packages/parsec.$BENCHMARK.pkgconf" ]; then
        source "$PARSECDIR/config/packages/parsec.$BENCHMARK.pkgconf"
        SUITE="parsec"
    elif [ -f "$PARSECDIR/config/packages/splash2x.$BENCHMARK.pkgconf" ]; then
        source "$PARSECDIR/config/packages/splash2x.$BENCHMARK.pkgconf"
        SUITE="splash2x"
    else
        echo "Couldn't find package: $1"
        exit 1
    fi
else
    echo "Couldn't find package: $1"
    exit 1
fi

# Filter to only allow apps and kernels
if [[ ! "apps kernels" =~ ( |^)${pkg_group}( |$) ]]; then
    echo "Only apps and kernels can be run!"
    exit 1
fi

# Navigate to benchmark directory
if [[ $SUITE == "parsec" ]]; then
    cd "$PARSECDIR/pkgs/$pkg_group/$BENCHMARK"
elif [[ $SUITE == "splash2x" ]]; then
    cd "$PARSECDIR/ext/splash2x/$pkg_group/$BENCHMARK"
fi

# Obtain launch parameters
source "parsec/$INPUTSIZE.runconf" || { 
    echo "Use one of the following:"
    cd parsec
    ls *.runconf
    exit 1
}
PROG="$(pwd)/inst/$PARSECPLAT/$run_exec"

# Prepare test input
rm -fr run
mkdir run
cd run
if [ -f "../inputs/input_$INPUTSIZE.tar" ]; then
    tar -xvf "../inputs/input_$INPUTSIZE.tar" > /dev/null
fi

# Run benchmark and record trace
mkdir -p $OUTDIR
if [[ $SUITE == "parsec" ]]; then
    "$GEMDIR/build/X86/gem5.fast" "$TOOLSDIR/gem5_config.py" $NCPUS $PROG $run_args \
        | $TOOLSDIR/extractor "$OUTDIR/$BENCHMARK.$INPUTSIZE"
elif [[ $SUITE == "splash2x" ]]; then
    cat $PROG | sed -rz 's/\n+$//' | head -n -2 > ./run_vars.sh
    source ./run_vars.sh $NTHREADS $INPUTSIZE
    if [[ $PROGARGS =~ '<' ]]; then
        "$GEMDIR/build/X86/gem5.fast" "$TOOLSDIR/gem5_config.py" $NCPUS $PROG $NTHREADS < $INPUTFILE \
            | $TOOLSDIR/extractor "$OUTDIR/$BENCHMARK.$INPUTSIZE"
    else
        "$GEMDIR/build/X86/gem5.fast" "$TOOLSDIR/gem5_config.py" $NCPUS $RUN \
            | $TOOLSDIR/extractor "$OUTDIR/$BENCHMARK.$INPUTSIZE"
    fi
fi

# Final message
echo "Trace file saved to $OUTDIR/$BENCHMARK.$INPUTSIZE.bin"
