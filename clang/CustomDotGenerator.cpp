#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Lex/Preprocessor.h"
#include <string>
#include <unordered_map>

using namespace clang;
using namespace clang::tooling;

//Class used to generate the node ID's for the dot file.
class NodeIDGenerator {
private:
  static int id;

public:
  static int getNextID() 
  {
    return id++;
  }
};

//First node ID is 1000
int NodeIDGenerator::id = 1000;

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
    //llvm::errs() << "Visiting a Decl " << Decloration->getDeclKindName() << "\n";
    //If it is a Parameter Object, we handle its output
    if(isa<ParmVarDecl>(Decloration)) {
      const ParmVarDecl *PVD = dyn_cast_or_null<ParmVarDecl>(Decloration);
      llvm::errs() << GetNodeId(Decloration) << " " << getParmVarDeclNodeString(PVD) << "\n";

    }
    //if it s just a variable decl that isnt a parameter...
    else if(isa<VarDecl>(Decloration)) {
      const VarDecl *VD = dyn_cast_or_null<VarDecl>(Decloration);
      llvm::errs() << GetNodeId(Decloration) << " " << getVarDeclNodeString(VD) << "\n";
    }
    //Functions point downward in the digraph to a bunch of nodes.  We itterate through them and draw the connections
    else if(isa<FunctionDecl>(Decloration)) {
      const FunctionDecl *FD = dyn_cast_or_null<FunctionDecl>(Decloration);
      llvm::errs() << GetNodeId(Decloration) << " " <<  getFunctionDeclNodeString(FD) << "\n";
      // Gater the params as an ArrayRef
      ArrayRef<ParmVarDecl * > params = FD->parameters();
      //Itterating through...
      for(auto p = params.begin(); p != params.end(); ++p) {
        //Insert the node if it doesnt exist yet.
        if (node_map.find(GetNodeName(*p)) == node_map.end()) {
          InsertNode(*p);
        }
        // TODO make sure we connect the function to the rest of the subgraph
        llvm::errs() << GetNodeId(Decloration) << "->" << GetNodeId(*p) << ";\n";
      }
    } else {
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
      llvm::errs() << GetNodeId(Statment) << " " << getIntegerLiteralNodeString(IL) << "\n";
    } else if (isa<BinaryOperator>(Statment)) {
      const BinaryOperator *BO = dyn_cast_or_null<BinaryOperator>(Statment);
      llvm::errs() << GetNodeId(Statment) << " " << getBinaryOperatorNodeString(BO) << "\n";
    } else if (isa<IfStmt>(Statment)) {
      const IfStmt *IS = dyn_cast_or_null<IfStmt>(Statment);
      llvm::errs() << GetNodeId(Statment) << " " << getIfStmtNodeString(IS) << "\n";
    } else if (isa<CompoundStmt>(Statment)) {
      const CompoundStmt *CS = dyn_cast_or_null<CompoundStmt>(Statment);
      llvm::errs() << GetNodeId(Statment) << " " << getCompoundStmtNodeString(CS) << "\n";
    } else if (isa<DeclStmt>(Statment)) {
      const DeclStmt *DS = dyn_cast_or_null<DeclStmt>(Statment);
      llvm::errs() << GetNodeId(Statment) << " " << getDeclStmtNodeString(DS) << "\n";
    } else if (isa<DeclRefExpr>(Statment)) {
      const DeclRefExpr *DRE = dyn_cast_or_null<DeclRefExpr>(Statment);
      llvm::errs() << GetNodeId(Statment) << " " << getDeclRefExprNodeString(DRE) << "\n";
    } else if (isa<CallExpr>(Statment)) {
      const CallExpr *CE = dyn_cast_or_null<CallExpr>(Statment);
      llvm::errs() << GetNodeId(Statment) << " " << getCallExprNodeString(CE) << "\n";
    } else if (isa<ImplicitCastExpr>(Statment)) {
      const ImplicitCastExpr *ICE = dyn_cast_or_null<ImplicitCastExpr>(Statment);
      //Commented out beacuse the masters thesis doesnt seems to do anything with this?  Not sure why
      //llvm::errs() << GetNodeId(Statment) << " " << getImplicitCastExprNodeString(ICE) << "\n";
    } else if (isa<ArraySubscriptExpr>(Statment)) {
      const ArraySubscriptExpr *ACE = dyn_cast_or_null<ArraySubscriptExpr>(Statment);
      llvm::errs() << GetNodeId(Statment) << " " << getArraySubscriptExprNodeString(ACE) << "\n";
    }
    else {
      llvm::errs() << Statment->getStmtClassName() << "\n";
    }
    // Itterating through all of my children, and drawing the connections
    for (Stmt::child_range I = Statment->children(); I; ++I) {
      if (*I) {
        if (node_map.find(GetNodeName(*I)) == node_map.end()) {
          InsertNode(*I);
        }
        //llvm::errs() << "\t" << I->getStmtClassName() << "\n";
        if (GetNodeId(Statment) != GetNodeId(*I)) {
          llvm::errs() << GetNodeId(Statment) << "->" << GetNodeId(*I) << ";\n";
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
    return "Decl l:c";
  }

  std::string GetNodeName(Stmt *Statment) {
    FullSourceLoc full_loc = Context->getFullLoc((Statment)->getLocStart());
    if (full_loc.isValid()) {
      return "Stmt:" + std::to_string(full_loc.getSpellingLineNumber()) + ":" + std::to_string(full_loc.getSpellingColumnNumber());
    }
    //For some reason we need this because the valid check fails on the first try
    return "Stmt l:c";
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
    return "[ shape=record , label=\"FucntionDecl\" , name = \"" + name + "\" , type = \"RETURNTYPE\"];";
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
    llvm::errs() << "digraph unnamed { \n";
    runToolOnCode(new CustomDotGeneratorAction, argv[1]);
    llvm::errs() << "}\n";
}