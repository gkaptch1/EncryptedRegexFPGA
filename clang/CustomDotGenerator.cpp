#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Lex/Preprocessor.h"
#include <string>

using namespace clang;
using namespace clang::tooling;

class CustomDotGeneratorVisitor 
  : public RecursiveASTVisitor<CustomDotGeneratorVisitor> {
public:
  explicit CustomDotGeneratorVisitor(ASTContext *Context)
    : Context(Context) {}

  bool VisitDecl(Decl *Decloration) {
    llvm::errs() << "Visiting a Decl " << Decloration->getDeclKindName() << "\n";
    if(isa<ParmVarDecl>(Decloration)) {
      const ParmVarDecl *PVD = dyn_cast_or_null<ParmVarDecl>(Decloration);
      llvm::errs() << PVD->getDeclKindName() << " is " << PVD->getDeclName() << ". \n";

    }
    if(isa<FunctionDecl>(Decloration)) {
      const FunctionDecl *FD = dyn_cast_or_null<FunctionDecl>(Decloration);
      ArrayRef<ParmVarDecl * > params = FD->parameters();
      for(auto p = params.begin(); p != params.end(); ++p) {
        llvm::errs() << "\t" << (*p)->getDeclName() << "\n";
      }
      /*
      for(FunctionDecl::param_const_range Ittr = FD->params(); Ittr; ++Ittr) 
        if(*Ittr)
          llvm::outs() << "\t" << Ittr->getDeclName();
          */
    }
    return true;
  }

  bool VisitStmt(Stmt *Statment) {
    //Print out all of my children with a tab
    llvm::outs() << Statment->getStmtClassName() << "\n";
    for (Stmt::child_range I = Statment->children(); I; ++I)
      if (*I)
        llvm::outs() << "\t" << I->getStmtClassName() << "\n";
  }

  bool VisitNamedDecl( NamedDecl *Declaration) {
    llvm::outs() << "Decl Name: " << Declaration->getQualifiedNameAsString() << "\n";
    return true;
  }

  //bool TransverseDecl(Decl *Decloration) {return true;}

private:
  ASTContext *Context;
};

class CustomDotGeneratorConsumer : public ASTConsumer {
ASTContext *Context;
public:
  explicit CustomDotGeneratorConsumer(ASTContext *Context)
    : Visitor(Context) {}

  /*bool HandleTopLevelDecl(DeclGroupRef D) override {
    for (DeclGroupRef::iterator I = D.begin(), E = D.end(); I != E; ++I)
      HandleTopLevelSingleDecl(*I);
    return true;
  }*/

  //void HandleTopLevelSingleDecl(Decl *D);

  virtual void HandleTranslationUnit(clang::ASTContext &Context) {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }

private:
  CustomDotGeneratorVisitor Visitor;
};


//The actual frontend action object.  It generats a ASTconsumer to consume the tree
class CustomDotGeneratorAction : public clang::ASTFrontendAction {
public:
  virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
    clang::CompilerInstance &Compiler, llvm::StringRef InFile) {

    return std::unique_ptr<clang::ASTConsumer>(
        new CustomDotGeneratorConsumer(&Compiler.getASTContext()));
  }
};

/*
void CustomDotGeneratorConsumer::HandleTopLevelSingleDecl(Decl *D) {
	//TODO Limits the things we are actually priniting to methods and functions...
  D->print(llvm::errs());
  llvm::errs() << '\n';

  if (isa<FunctionDecl>(D) || isa<ObjCMethodDecl>(D)) {
    if (Stmt *Body = D->getBody()) {
      llvm::errs() << '\n';
      Body->viewAST();
      llvm::errs() << '\n';
    } else {

    }
  }
}
*/
/*
std::unique_ptr<ASTConsumer> clang::CreateDeclContextPrinter() {
  return llvm::make_unique<DeclContextPrinter>();
}
*/

int main(int argc, char **argv) {
    //TODO make this not shitty and actually read in a file
    runToolOnCode(new CustomDotGeneratorAction, argv[1]);
}