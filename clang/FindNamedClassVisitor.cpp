#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Lex/Preprocessor.h"
#include <string>
//#include "clang/Tooling/CommonOptionsParser.h"
//#include "llvm/Support/CommandLine.h"

using namespace clang;
using namespace clang::tooling;

class NodeIDGenerator {
private:
  static int id;

public:
  static int getNextID() 
  {
    return id++;
  }
};

int NodeIDGenerator::id = 1000;

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

//  bool VisitStmt( Stmt *Statment ) {
  //}

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

class CustomTokenDumperAction : public clang::PreprocessorFrontendAction {
protected:
  virtual void ExecuteAction() {
   //CompilerInstance &CI = getCompilerInstance();
   Preprocessor &PP = getCompilerInstance().getPreprocessor();
   SourceManager &SM = PP.getSourceManager();
 
   // Start lexing the specified input file.
   const llvm::MemoryBuffer *FromFile = SM.getBuffer(SM.getMainFileID());
   Lexer RawLex(SM.getMainFileID(), FromFile, SM, PP.getLangOpts());
   RawLex.SetKeepWhitespaceMode(true);
 
   Token RawTok;
   RawLex.LexFromRawLexer(RawTok);
   while (RawTok.isNot(tok::eof)){
    std::string spelling = PP.getSpelling(RawTok);
    //We want to strip out all the global and kernel annotations.  
     if(spelling.compare(0,8,"__global") == 0 || spelling.compare(0,8,"__kernel") == 0) {
      //Rips out the space after the annotation
       RawLex.LexFromRawLexer(RawTok);
       RawLex.LexFromRawLexer(RawTok);
       continue;
     }
     //PP.DumpToken(RawTok, true);
     llvm::errs() << spelling;
     //llvm::errs() << "\n";
     RawLex.LexFromRawLexer(RawTok);
   }

   //TODO FIGURE OUT HOW TO PASS THIS TO THE AST PARSER!! 
   /*
   raw_ostream *OS = CI.createDefaultOutputFile(false, getCurrentFile());
   if (!OS) return;
 
   DoPrintPreprocessedInput(CI.getPreprocessor(), OS,
                            CI.getPreprocessorOutputOpts());
                            */
  }
};

//Rewriter rewriter;

int main(int argc, char **argv) {
    //CommonOptionsParser op(argc, argv);
    //ClangTool Tool(op.getCompilations(), op.getSourcePathList());
    //int result = Tool.run(newFrontendActionFactory<CustomTokenDumperAction>());
    //runToolOnCode(new ASTDumpAction, argv[1]);
    runToolOnCode(new ASTViewAction, argv[1]);
    //rewriter.getEditBuffer(rewriter.getSourceMgr().getMainFileID()).write(errs());
}