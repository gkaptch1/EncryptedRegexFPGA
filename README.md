# EncryptedRegexFPGA

##Some important links

AutoESL // ViVado HLS Download
http://www.xilinx.com/tools/autoesl_instructions.htm
http://www.xilinx.com/support/download.html

Clang AST
http://clang.llvm.org/docs/IntroductionToTheClangAST.html  -- Video is pretty good

py-graph-tool
~~~~~~~~~~~~~~~~~~~~~~~~~~~
sudo port install py-graph-tool
~~~~~~~~~~~~~~~~~~~~~~~~~~~~
or
~~~~~~~~~~~~~~~~~~~~~~~~~~~
brew tap homebrew/science
brew install graph-tool
~~~~~~~~~~~~~~~~~~~~~~~~~~~

##Translator needs

| What | How (Source)| How (AutoESL)| Why |
| ---- | ------------| -------------| --- |
| Arbitrary precision integer and fixed point libraries to minimize bus width| Find variables with size bounded by code (e.g. loop condition variables)| C: ap_[u]intN where N is bit width| |
| Streams| Not sure | C++ Only? hsl::stream<type> | For implementing streaming I/O like an incoming signal. A simple filter could check stream.full() |
| Barriers | Find time dependent sections of code | Set bit in bit array high from each component/ core/ kernel. When all bits are high in the driver, send a signal back to each kernel to continue execution | |
| LFSR generation for counters? | Find counter variables | Could translate traditional counters to LFSRs using techniques from FPGA Syn. Lookup LFSR xor indices -> loop until we find end state -> create LFSR and send signal when end state reached | Space minimization. Latency minimization (1 clock cycle vs multiple). |



FOR LINUX

Graphviz and gv install
~~~~~~~~~~~~~~~~~~~~~~~
http://stackoverflow.com/questions/10962845/stmtviewast-is-only-available-in-debug-builds-on-systems-with-graphviz-or-gv

clang debug install
~~~~~~~~~~~~~~~~~~~~~~~
http://clang.llvm.org/get_started.html

py-graph-tool
~~~~~~~~~~~~~~~~~~
apt-get

FrontEnd Action Guide
~~~~~~~~~~~~~~~~~~~~~~~~
http://clang.llvm.org/docs/RAVFrontendAction.html