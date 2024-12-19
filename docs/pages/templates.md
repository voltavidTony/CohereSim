# Template file generation

[TOC]

The template file generation script is designed to automate the boilerplate code. It generates the interface, allowing the programmer to jump straight to programming the implementation. The three different templates are explained below.

## Template options

### 1 Coherence protocol source file

The name of the coherence protocol is in `TitleCase`. This will create a source and header file pair in the `src/coherence` directory from the template files. The class will contain blank (default behavior) methods ready to accept the concrete implementation of the specific coherence protocol.

Note:: The `CoherenceProtocol` base class is abstract, specifically, the `CoherenceProtocol::PrRd`, `CoherenceProtocol::PrWr`, `CoherenceProtocol::BusRd`, and `CoherenceProtocol::isWriteBackNeeded` methods require implementation in the specific coherence protocol. The remaining methods are optional to cater to different coherence protocols and the bus messages they use. For instance, the `Dragon` protocol only reimplements the `Dragon::BusUpdt` and `Dragon::doesDirtySharing` methods.

### 2 Replacement policy source file

The name of the replacement policy is in `TitleCase`. This will create a source and header file pair in the `src/replacement` directory from the template files. The class will contain blank (default behavior) methods ready to accept the concrete implementation of the specific replacement policy.

Note:: The `ReplacementPolicy` base class contains default implementations for all methods, so if the specific replacement policy does not require one of them to operate correctly, it can be entirely removed from the header and source files. For instance, the random replacement (`RR`) policy does not reimplement the `ReplacementPolicy::touch` method.

### 3 Textbook mode source file

The name of the textbook mode cache is in `TitleCase`. This will create a source and header file pair in the `src/textbook` directory from the template files. The class will contain blank (default behavior) methods ready to accept the concrete implementation of the specific replacement policy.

Note:: The `TextbookMode` base class is abstract, most methods require implementation in the specific textbook mode cache. Only `TextbookMode::issueBusMsg` and `TextbookMode::getLineState` have default implementations making them optional in the specific textbook mode cache. They exist to cater to the different needs of the policy/protocol used. For instance, the TextbookModeReplacer class does not issue bus messages and thus does not implement the `TextbookMode::issueBusMsg` method.

### 4 Generic source file

This mode is the simplest: Specify a file name in `snake_case`, and a blank (minimal) source and header file pair using that file name appears in the `src` directory.

## Run the trace generation script

Note:: This script is located in the `template` directory, but can be run from anywhere.

Usage:
1. `./mksrc.sh coherence <class_name> <states..>`
2. `./mksrc.sh replacement|textbook <class_name>`
3. `./mksrc.sh <file_name>`

- `class_name`: The name of the new policy/protocol (TitleCase)
- `file_name`: The file name of the generic source files (snake_case)
- `states..`: The list of states in the coherence protocol's state transition graph. Chose at least two of `I`, `D`, `E`, `M`, `V`, `O`, `S`, `Sc`, and `Sm`
