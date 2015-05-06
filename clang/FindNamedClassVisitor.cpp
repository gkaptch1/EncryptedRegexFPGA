#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/Tooling.h"
//#include "clang/Tooling/CommonOptionsParser.h"
//#include "llvm/Support/CommandLine.h"

using namespace clang;

class FindNamedClassVisitor
  : public RecursiveASTVisitor<FindNamedClassVisitor> {
public:
  explicit FindNamedClassVisitor(ASTContext *Context)
    : Context(Context) {}

  bool VisitNamedDecl( NamedDecl *Declaration) {
    llvm::outs() << "Decl Name: " << Declaration->getQualifiedNameAsString() << "\n";
    return true;
  }

  bool VisitAttr( Attr *Attribute) {
    llvm::outs() << "Attr Name: " << Attribute->getSpelling() << "\n";
    return true;
  }

  bool VisitType( Type *T) {
    llvm::outs() << "Type Name: " << T->getTypeClassName() << "\n";
    return true;
  }

private:
  ASTContext *Context;
};

class FindNamedClassConsumer : public clang::ASTConsumer {
public:
  explicit FindNamedClassConsumer(ASTContext *Context)
    : Visitor(Context) {}

  virtual void HandleTranslationUnit(clang::ASTContext &Context) {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }
private:
  FindNamedClassVisitor Visitor;
};

class FindNamedClassAction : public clang::ASTFrontendAction {
public:
  virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
    clang::CompilerInstance &Compiler, llvm::StringRef InFile) {



    return std::unique_ptr<clang::ASTConsumer>(
        new FindNamedClassConsumer(&Compiler.getASTContext()));
  }
};

class CustomTokenDumperAction : public clang::DumpRawTokensAction {
public:


};

int main(int argc, char **argv) {
  if (argc > 1) {
    clang::tooling::runToolOnCode(new DumpRawTokensAction, argv[1]);
    clang::tooling::runToolOnCode(new FindNamedClassAction, argv[1]);
  }
}