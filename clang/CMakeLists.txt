set(LLVM_LINK_COMPONENTS support)

set(LLVM_USED_LIBS clangTooling clangBasic clangAST)

add_clang_executable(find-class-decls FindNamedClassVisitor.cpp)

target_link_libraries(find-class-decls 
  clangTooling
  clangBasic
  clangASTMatchers
  )