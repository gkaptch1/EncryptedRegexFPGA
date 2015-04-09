# EncryptedRegexFPGA

Some important links
~~~~~~~~~~~~~~~~~~~~~~~~~~~

AutoESL // ViVado HLS Download
http://www.xilinx.com/tools/autoesl_instructions.htm
http://www.xilinx.com/support/download.html

Clang AST
http://clang.llvm.org/docs/IntroductionToTheClangAST.html  -- Video is pretty good

py-graph-tool
sudo port install py-graph-tool

Translator needs
|What|How|Notes|
|----|---|-----|
|Arbitrary precision integer and fixed point libraries to minimize bus width|Find variables with size bounded by code (e.g. loop condition variables) | C: ap_[u]intN where N is bit width|
||||
||||
