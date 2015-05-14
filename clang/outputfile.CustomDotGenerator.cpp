#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Lex/Preprocessor.h"

#include "llvm/Support/Host.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"

#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/AST/ASTContext.h"
#include "clang/Parse/Parser.h"
#include "clang/Parse/ParseAST.h"


#include <string>
#include <unordered_map>
//#include <iostream>
//#include <fstream>

using namespace clang;
using namespace clang::tooling;

//Class used to generate the node ID's for the dot file.
class NodeIDGenerator {
public:
  static int getNextID() 
  {
    return id++;
  }

private:
  static int id;
};

//First node ID is 1000
int NodeIDGenerator::id = 1000;

class DotFileWriter {
public:
  static bool OpenFile(std::string file_name) {
    if (file_open) {
      return false;
    }
   // output_file.open(file_name);
    return true;
  }

  static bool WriteToFile(std::string to_wrtie) {
    if (!file_open) {
      return false;
    }
  //  output_file << to_wrtie;
    return true;
  }

  static bool CloseFile() {
    if (!file_open) {
      return false;
    }
//    output_file.close();
    return true;
  }
private:
  static bool file_open;
//  static std::ofstream output_file;

};

// bool DotFileWriter::file_open = false;

class CustomDotGeneratorVisitor 
  : public RecursiveASTVisitor<CustomDotGeneratorVisitor> {
public:
  explicit CustomDotGeneratorVisitor(ASTContext *Context)
    : Context(Context) {}

  bool VisitDecl(Decl *Decloration) {

    //If we are visiting something that somehow isnt in the digraph yet, we insert it
    if (node_map.find(GetNodeName(Decloration)) == node_map.end()) {
      InsertNode(Decloration);
    }
    //If it is a Parameter Object, we handle its output
    if(isa<ParmVarDecl>(Decloration)) {
      const ParmVarDecl *PVD = dyn_cast_or_null<ParmVarDecl>(Decloration);
      DotFileWriter::WriteToFile(GetNodeId(Decloration) + " " + getParmVarDeclNodeString(PVD) + "\n");

    }
    //if it s just a variable decl that isnt a parameter...
    else if(isa<VarDecl>(Decloration)) {
      const VarDecl *VD = dyn_cast_or_null<VarDecl>(Decloration);
      DotFileWriter::WriteToFile(GetNodeId(Decloration) + " " + getVarDeclNodeString(VD) + "\n");
    }
    //Functions point downward in the digraph to a bunch of nodes.  We itterate through them and draw the connections
    else if(isa<FunctionDecl>(Decloration)) {
      const FunctionDecl *FD = dyn_cast_or_null<FunctionDecl>(Decloration);
      DotFileWriter::WriteToFile(GetNodeId(Decloration) + " " +  getFunctionDeclNodeString(FD) + "\n");
      // Gater the params as an ArrayRef
      ArrayRef<ParmVarDecl * > params = FD->parameters();
      //Itterating through...
      for(auto p = params.begin(); p != params.end(); ++p) {
        //Insert the node if it doesnt exist yet.
        if (node_map.find(GetNodeName(*p)) == node_map.end()) {
          InsertNode(*p);
        }
        DotFileWriter::WriteToFile(GetNodeId(Decloration) + "->" + GetNodeId(*p) + ";\n");
      }
      //Get the top level stmt node in the function body
      if(FD->hasBody()) {
        Stmt *Body = FD->getBody();
        InsertNode(Body); 
        DotFileWriter::WriteToFile(GetNodeId(Decloration) + "->" + GetNodeId(Body) + ";\n");
      }

    } else {
      //Things we dont care about are listed here
      if(isa<TranslationUnitDecl>(Decloration)) {
        //We dont want to print anything if we are considering a TranslationUnit
        return true;
      }
      // Mostly for debug purposes.  Making sure we didnt miss anything...
      llvm::errs() << "Visiting an unhandled Decl " << Decloration->getDeclKindName() << "\n";
    }
    return true;
  }

  bool VisitStmt(Stmt *Statment) {
    //Insert the node into the map if it has yet to happen
    if (node_map.find(GetNodeName(Statment)) == node_map.end()) {
      InsertNode(Statment);
    }
    //Handle all the various possible statment strings we might need
    // Each block casts the stamtnet to that type then hands off the string building to a helper method defined below
    if( isa<IntegerLiteral>(Statment) ) {
      const IntegerLiteral *IL = dyn_cast_or_null<IntegerLiteral>(Statment);
      DotFileWriter::WriteToFile(GetNodeId(Statment) + " " + getIntegerLiteralNodeString(IL) + "\n");
    } else if (isa<BinaryOperator>(Statment)) {
      const BinaryOperator *BO = dyn_cast_or_null<BinaryOperator>(Statment);
      DotFileWriter::WriteToFile(GetNodeId(Statment) + " " + getBinaryOperatorNodeString(BO) + "\n");
    } else if (isa<IfStmt>(Statment)) {
      const IfStmt *IS = dyn_cast_or_null<IfStmt>(Statment);
      DotFileWriter::WriteToFile(GetNodeId(Statment) + " " + getIfStmtNodeString(IS) + "\n");
    } else if (isa<CompoundStmt>(Statment)) {
      const CompoundStmt *CS = dyn_cast_or_null<CompoundStmt>(Statment);
      DotFileWriter::WriteToFile(GetNodeId(Statment) + " " + getCompoundStmtNodeString(CS) + "\n");
    } else if (isa<DeclStmt>(Statment)) {
      const DeclStmt *DS = dyn_cast_or_null<DeclStmt>(Statment);
      DotFileWriter::WriteToFile(GetNodeId(Statment) + " " + getDeclStmtNodeString(DS) + "\n");
    } else if (isa<DeclRefExpr>(Statment)) {
      const DeclRefExpr *DRE = dyn_cast_or_null<DeclRefExpr>(Statment);
      DotFileWriter::WriteToFile(GetNodeId(Statment) + " " + getDeclRefExprNodeString(DRE) + "\n");
    } else if (isa<CallExpr>(Statment)) {
      const CallExpr *CE = dyn_cast_or_null<CallExpr>(Statment);
      DotFileWriter::WriteToFile(GetNodeId(Statment) + " " + getCallExprNodeString(CE) + "\n");
    } else if (isa<ImplicitCastExpr>(Statment)) {
      const ImplicitCastExpr *ICE = dyn_cast_or_null<ImplicitCastExpr>(Statment);
      //Commented out beacuse the masters thesis doesnt seems to do anything with this?  Not sure why
      //llvm::errs() << GetNodeId(Statment) << " " << getImplicitCastExprNodeString(ICE) << "\n";
    } else if (isa<ArraySubscriptExpr>(Statment)) {
      const ArraySubscriptExpr *ACE = dyn_cast_or_null<ArraySubscriptExpr>(Statment);
      DotFileWriter::WriteToFile(GetNodeId(Statment) + " " + getArraySubscriptExprNodeString(ACE) + "\n");
    }
    else {
      llvm::errs() << "Visiting unhandeled Stmt " << Statment->getStmtClassName() << "\n";
    }
    // Itterating through all of my children, and drawing the connections
    for (Stmt::child_range I = Statment->children(); I; ++I) {
      if (*I) {
        if (node_map.find(GetNodeName(*I)) == node_map.end()) {
          InsertNode(*I);
        }
        //llvm::errs() << "\t" << I->getStmtClassName() << "\n";
        if (GetNodeId(Statment) != GetNodeId(*I)) {
          DotFileWriter::WriteToFile(GetNodeId(Statment) + "->" + GetNodeId(*I) + ";\n");
        }
      }
    }
    return true;
  }

private:
  ASTContext *Context;
  //Map of unique IDS to the node ID's to be used in the dot file
  std::unordered_map<std::string,int> node_map = std::unordered_map<std::string, int>();

  // --------------------------- For Map Manipulations. Functions can either take Stmts or Decls

  std::string InsertNode(Decl *Decloration) {
    //If the node already has an ID in the map, we simply return the value
    if( node_map.find(GetNodeName(Decloration)) != node_map.end()) {
      return "Node" + std::to_string(node_map.at(GetNodeName(Decloration)));
    }
    //Generate next Id
    int node_id = NodeIDGenerator::getNextID();
    //Insert into the map
    node_map.insert(std::make_pair<std::string,double>(GetNodeName(Decloration),node_id));
    //llvm::errs() << "\t INSERTING NODE " << GetNodeName(Decloration) << " NODE" << std::to_string(node_id) << "\n";
    return "Node" + std::to_string(node_id);
  }

  //We duplicate the above method from Statments
  std::string InsertNode(Stmt *Statment) {
    if( node_map.find(GetNodeName(Statment)) != node_map.end()) {
      return "Node" + std::to_string(node_map.at(GetNodeName(Statment)));
    }
    int node_id = NodeIDGenerator::getNextID();
    node_map.insert(std::make_pair<std::string,double>(GetNodeName(Statment),node_id));
    //llvm::errs() << "\t INSERTING NODE " << GetNodeName(Statment) << " NODE" << std::to_string(node_id) << "\n";
    return "Node" + std::to_string(node_id);
  }

  //Wrapper function for readability.  Logic implemented above
  std::string GetNodeId(Decl *Decloration) {
    return InsertNode(Decloration);
  }

  //Wrapper function for readability.  Logic implemented above
  std::string GetNodeId(Stmt *Statment) {
    return InsertNode(Statment);
  }

  std::string GetNodeName(Decl *Decloration) {
    FullSourceLoc full_loc = Context->getFullLoc((Decloration)->getLocStart());
    if (full_loc.isValid()) {
      return "Decl:" + std::to_string(full_loc.getSpellingLineNumber()) + ":" + std::to_string(full_loc.getSpellingColumnNumber());
    }
    //For some reason we need this because the valid check fails on the first try
    return "Decl:l:c";
  }

  std::string GetNodeName(Stmt *Statment) {
    FullSourceLoc full_loc = Context->getFullLoc((Statment)->getLocStart());
    if (full_loc.isValid()) {
      return "Stmt:" + std::to_string(full_loc.getSpellingLineNumber()) + ":" + std::to_string(full_loc.getSpellingColumnNumber());
    }
    //For some reason we need this because the valid check fails on the first try
    return "Stmt:l:c";
  }

  // ------------------------  The rest of the class in helped methods defining the strings we want to ouput into the dot file.

  //------------------------ For Decls

  std::string getParmVarDeclNodeString(const ParmVarDecl *PVD) {
    std::string name = PVD->getDeclName().getAsString();
    std::string type = PVD->getType().getAsString();
    return "[ shape=record , label=\"ParmVarDecl\" , name = \"" + name + "\" , type = \"" + type + "\"];";
  }

  std::string getFunctionDeclNodeString(const FunctionDecl *FD) {
    std::string name = FD->getQualifiedNameAsString();
    std::string returntype = FD->getReturnType().getAsString();
    return "[ shape=record , label=\"FucntionDecl\" , name = \"" + name + "\" , type = \"" + returntype + "\"];";
  }

  std::string getVarDeclNodeString(const VarDecl *VD) {
    std::string name = VD->getDeclName().getAsString();
    std::string type = VD->getType().getAsString();
    //std::string value = std::to_string(VD->evaluateValue());
    return "[ shape=record , label=\"VarDecl\" , name = \"" + name + "\" , type = \"" + type + "\" , value = \"VALUE\" ];";
  }

  //--------------------------- For Statments 

  std::string getIntegerLiteralNodeString(const IntegerLiteral *IL) {
    std::string value = std::to_string( (int) (IL->getValue()).bitsToDouble());
    return "[ shape=record , label=\"IntegerLiteral\" , value = \"" + value + "\"];";
  }

  std::string getBinaryOperatorNodeString(const BinaryOperator *BO) {
    std::string value = BO->getOpcodeStr();
    return "[ shape=record , label=\"BinaryOperator\" , value = \"" + value + "\"];";
  }

  std::string getCompoundStmtNodeString(const CompoundStmt *CS) {
    return "[ shape=record , label=\"CompoundStmt\" ];";
  }

  std::string getIfStmtNodeString(const IfStmt *IS) {
    return "[ shape=record , label=\"IfStmt\" ];";
  }

  std::string getDeclStmtNodeString(const DeclStmt *DS) {
    return "[ shape=record , label=\"DeclStmt\" ];";
  }

  std::string getDeclRefExprNodeString(const DeclRefExpr *DRE) {
    std::string name = (DRE->getNameInfo()).getAsString();
    return "[ shape=record , label=\"DeclRefExpr\" , name = \"" + name + "\"];";  
  }

  std::string getCallExprNodeString(const CallExpr *CE) {
    return "[ shape=record , label=\"CallExpr\" ];";
  }

  std::string getImplicitCastExprNodeString(const ImplicitCastExpr *ICE) {
    //Not sure why this is nothing...
    return "";
  }

  std::string getArraySubscriptExprNodeString(const ArraySubscriptExpr *ACE) {
    return "[ shape=record , label=\"ArraySubscriptExpr\" ];";
  }

};

class CustomDotGeneratorConsumer : public ASTConsumer {
ASTContext *Context;
public:
  explicit CustomDotGeneratorConsumer(ASTContext *Context)
    : Visitor(Context) {}

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


int main(int argc, char **argv) {
    //TODO make this not shitty and actually read in a file
    if( argc >= 2 ) {
      DotFileWriter::OpenFile(argv[2]);
      DotFileWriter::WriteToFile( "digraph unnamed { \n");
      //runToolOnCode(new CustomDotGeneratorAction, argv[1]);

      CompilerInstance compiler_instance;
      DiagnosticOptions diagnosticOptions;
      compiler_instance.createDiagnostics();

      // Initialize target info with the default triple for our platform.
      auto TO = std::make_shared<TargetOptions>();
      TO->Triple = llvm::sys::getDefaultTargetTriple();
      TargetInfo *TI =
          TargetInfo::CreateTargetInfo(compiler_instance.getDiagnostics(), TO);
      compiler_instance.setTarget(TI);

      // Set up the environment 
      compiler_instance.createFileManager();
      compiler_instance.createSourceManager(compiler_instance.getFileManager());
      compiler_instance.createPreprocessor(clang::TU_Complete);
      compiler_instance.getPreprocessorOpts().UsePredefines = false;
      compiler_instance.createASTContext();
      CustomDotGeneratorConsumer *astConsumer = new CustomDotGeneratorConsumer(&compiler_instance.getASTContext());
      compiler_instance.setASTConsumer(std::unique_ptr<ASTConsumer>(astConsumer));

      // Set the main file handled by the source manager to the input file.
      const FileEntry *FileIn = compiler_instance.getFileManager().getFile(argv[1]);
      compiler_instance.getSourceManager().setMainFileID(
        compiler_instance.getSourceManager().createFileID(FileIn, SourceLocation(), SrcMgr::C_User));
      compiler_instance.getDiagnosticClient().BeginSourceFile(
        compiler_instance.getLangOpts(), &compiler_instance.getPreprocessor());

      clang::ParseAST(compiler_instance.getPreprocessor(), astConsumer, compiler_instance.getASTContext());
      compiler_instance.getDiagnosticClient().EndSourceFile();

      DotFileWriter::WriteToFile("}\n");

    }
    return 0;
}