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

class CustomDotGeneratorConsumer : public ASTConsumer {
ASTContext *Context;
public:
	void Initialize(ASTContext &Context) override {
	  this->Context = &Context;
	}

	bool HandleTopLevelDecl(DeclGroupRef D) override {
	  for (DeclGroupRef::iterator I = D.begin(), E = D.end(); I != E; ++I)
	    HandleTopLevelSingleDecl(*I);
	  return true;
	}

	void HandleTopLevelSingleDecl(Decl *D);
};


void CustomDotGeneratorConsumer::HandleTopLevelSingleDecl(Decl *D) {
	//TODO Limits the things we are actually priniting to methods and functions...
  if (isa<FunctionDecl>(D) || isa<ObjCMethodDecl>(D)) {
    D->print(llvm::errs());
  
    if (Stmt *Body = D->getBody()) {
      llvm::errs() << '\n';
      Body->viewAST();
      llvm::errs() << '\n';
    }
  }
}

//===----------------------------------------------------------------------===//
/// DeclContextPrinter - Decl and DeclContext Visualization

class DeclContextPrinter : public ASTConsumer {
  raw_ostream& Out;
public:
  DeclContextPrinter() : Out(llvm::errs()) {}

  void HandleTranslationUnit(ASTContext &C) override {
    PrintDeclContext(C.getTranslationUnitDecl(), 4);
  }

  void PrintDeclContext(const DeclContext* DC, unsigned Indentation);
};

void DeclContextPrinter::PrintDeclContext(const DeclContext* DC,
                                          unsigned Indentation) {
  // Print DeclContext name.
  switch (DC->getDeclKind()) {
  case Decl::TranslationUnit:
    Out << "[translation unit] " << DC;
    break;
  case Decl::Namespace: {
    Out << "[namespace] ";
    const NamespaceDecl* ND = cast<NamespaceDecl>(DC);
    Out << *ND;
    break;
  }
  case Decl::Enum: {
    const EnumDecl* ED = cast<EnumDecl>(DC);
    if (ED->isCompleteDefinition())
      Out << "[enum] ";
    else
      Out << "<enum> ";
    Out << *ED;
    break;
  }
  case Decl::Record: {
    const RecordDecl* RD = cast<RecordDecl>(DC);
    if (RD->isCompleteDefinition())
      Out << "[struct] ";
    else
      Out << "<struct> ";
    Out << *RD;
    break;
  }
  case Decl::CXXRecord: {
    const CXXRecordDecl* RD = cast<CXXRecordDecl>(DC);
    if (RD->isCompleteDefinition())
      Out << "[class] ";
    else
      Out << "<class> ";
    Out << *RD << ' ' << DC;
    break;
  }
  case Decl::ObjCMethod:
    Out << "[objc method]";
    break;
  case Decl::ObjCInterface:
    Out << "[objc interface]";
    break;
  case Decl::ObjCCategory:
    Out << "[objc category]";
    break;
  case Decl::ObjCProtocol:
    Out << "[objc protocol]";
    break;
  case Decl::ObjCImplementation:
    Out << "[objc implementation]";
    break;
  case Decl::ObjCCategoryImpl:
    Out << "[objc categoryimpl]";
    break;
  case Decl::LinkageSpec:
    Out << "[linkage spec]";
    break;
  case Decl::Block:
    Out << "[block]";
    break;
  case Decl::Function: {
    const FunctionDecl* FD = cast<FunctionDecl>(DC);
    if (FD->doesThisDeclarationHaveABody())
      Out << "[function] ";
    else
      Out << "<function> ";
    Out << *FD;
    // Print the parameters.
    Out << "(";
    bool PrintComma = false;
    for (auto I : FD->params()) {
      if (PrintComma)
        Out << ", ";
      else
        PrintComma = true;
      Out << *I;
    }
    Out << ")";
    break;
  }
  case Decl::CXXMethod: {
    const CXXMethodDecl* D = cast<CXXMethodDecl>(DC);
    if (D->isOutOfLine())
      Out << "[c++ method] ";
    else if (D->isImplicit())
      Out << "(c++ method) ";
    else
      Out << "<c++ method> ";
    Out << *D;
    // Print the parameters.
    Out << "(";
    bool PrintComma = false;
    for (FunctionDecl::param_const_iterator I = D->param_begin(),
           E = D->param_end(); I != E; ++I) {
      if (PrintComma)
        Out << ", ";
      else
        PrintComma = true;
      Out << **I;
    }
    Out << ")";

    // Check the semantic DeclContext.
    const DeclContext* SemaDC = D->getDeclContext();
    const DeclContext* LexicalDC = D->getLexicalDeclContext();
    if (SemaDC != LexicalDC)
      Out << " [[" << SemaDC << "]]";

    break;
  }
  case Decl::CXXConstructor: {
    const CXXConstructorDecl* D = cast<CXXConstructorDecl>(DC);
    if (D->isOutOfLine())
      Out << "[c++ ctor] ";
    else if (D->isImplicit())
      Out << "(c++ ctor) ";
    else
      Out << "<c++ ctor> ";
    Out << *D;
    // Print the parameters.
    Out << "(";
    bool PrintComma = false;
    for (FunctionDecl::param_const_iterator I = D->param_begin(),
           E = D->param_end(); I != E; ++I) {
      if (PrintComma)
        Out << ", ";
      else
        PrintComma = true;
      Out << **I;
    }
    Out << ")";

    // Check the semantic DC.
    const DeclContext* SemaDC = D->getDeclContext();
    const DeclContext* LexicalDC = D->getLexicalDeclContext();
    if (SemaDC != LexicalDC)
      Out << " [[" << SemaDC << "]]";
    break;
  }
  case Decl::CXXDestructor: {
    const CXXDestructorDecl* D = cast<CXXDestructorDecl>(DC);
    if (D->isOutOfLine())
      Out << "[c++ dtor] ";
    else if (D->isImplicit())
      Out << "(c++ dtor) ";
    else
      Out << "<c++ dtor> ";
    Out << *D;
    // Check the semantic DC.
    const DeclContext* SemaDC = D->getDeclContext();
    const DeclContext* LexicalDC = D->getLexicalDeclContext();
    if (SemaDC != LexicalDC)
      Out << " [[" << SemaDC << "]]";
    break;
  }
  case Decl::CXXConversion: {
    const CXXConversionDecl* D = cast<CXXConversionDecl>(DC);
    if (D->isOutOfLine())
      Out << "[c++ conversion] ";
    else if (D->isImplicit())
      Out << "(c++ conversion) ";
    else
      Out << "<c++ conversion> ";
    Out << *D;
    // Check the semantic DC.
    const DeclContext* SemaDC = D->getDeclContext();
    const DeclContext* LexicalDC = D->getLexicalDeclContext();
    if (SemaDC != LexicalDC)
      Out << " [[" << SemaDC << "]]";
    break;
  }

  default:
    llvm_unreachable("a decl that inherits DeclContext isn't handled");
  }

  Out << "\n";

  // Print decls in the DeclContext.
  for (auto *I : DC->decls()) {
    for (unsigned i = 0; i < Indentation; ++i)
      Out << "  ";

    Decl::Kind DK = I->getKind();
    switch (DK) {
    case Decl::Namespace:
    case Decl::Enum:
    case Decl::Record:
    case Decl::CXXRecord:
    case Decl::ObjCMethod:
    case Decl::ObjCInterface:
    case Decl::ObjCCategory:
    case Decl::ObjCProtocol:
    case Decl::ObjCImplementation:
    case Decl::ObjCCategoryImpl:
    case Decl::LinkageSpec:
    case Decl::Block:
    case Decl::Function:
    case Decl::CXXMethod:
    case Decl::CXXConstructor:
    case Decl::CXXDestructor:
    case Decl::CXXConversion:
    {
      DeclContext* DC = cast<DeclContext>(I);
      PrintDeclContext(DC, Indentation+2);
      break;
    }
    case Decl::IndirectField: {
      IndirectFieldDecl* IFD = cast<IndirectFieldDecl>(I);
      Out << "<IndirectField> " << *IFD << '\n';
      break;
    }
    case Decl::Label: {
      LabelDecl *LD = cast<LabelDecl>(I);
      Out << "<Label> " << *LD << '\n';
      break;
    }
    case Decl::Field: {
      FieldDecl *FD = cast<FieldDecl>(I);
      Out << "<field> " << *FD << '\n';
      break;
    }
    case Decl::Typedef:
    case Decl::TypeAlias: {
      TypedefNameDecl* TD = cast<TypedefNameDecl>(I);
      Out << "<typedef> " << *TD << '\n';
      break;
    }
    case Decl::EnumConstant: {
      EnumConstantDecl* ECD = cast<EnumConstantDecl>(I);
      Out << "<enum constant> " << *ECD << '\n';
      break;
    }
    case Decl::Var: {
      VarDecl* VD = cast<VarDecl>(I);
      Out << "<var> " << *VD << '\n';
      break;
    }
    case Decl::ImplicitParam: {
      ImplicitParamDecl* IPD = cast<ImplicitParamDecl>(I);
      Out << "<implicit parameter> " << *IPD << '\n';
      break;
    }
    case Decl::ParmVar: {
      ParmVarDecl* PVD = cast<ParmVarDecl>(I);
      Out << "<parameter> " << *PVD << '\n';
      break;
    }
    case Decl::ObjCProperty: {
      ObjCPropertyDecl* OPD = cast<ObjCPropertyDecl>(I);
      Out << "<objc property> " << *OPD << '\n';
      break;
    }
    case Decl::FunctionTemplate: {
      FunctionTemplateDecl* FTD = cast<FunctionTemplateDecl>(I);
      Out << "<function template> " << *FTD << '\n';
      break;
    }
    case Decl::FileScopeAsm: {
      Out << "<file-scope asm>\n";
      break;
    }
    case Decl::UsingDirective: {
      Out << "<using directive>\n";
      break;
    }
    case Decl::NamespaceAlias: {
      NamespaceAliasDecl* NAD = cast<NamespaceAliasDecl>(I);
      Out << "<namespace alias> " << *NAD << '\n';
      break;
    }
    case Decl::ClassTemplate: {
      ClassTemplateDecl *CTD = cast<ClassTemplateDecl>(I);
      Out << "<class template> " << *CTD << '\n';
      break;
    }
    case Decl::OMPThreadPrivate: {
      Out << "<omp threadprivate> " << '"' << I << "\"\n";
      break;
    }
    default:
      Out << "DeclKind: " << DK << '"' << I << "\"\n";
      llvm_unreachable("decl unhandled");
    }
  }
}

std::unique_ptr<ASTConsumer> clang::CreateDeclContextPrinter() {
  return llvm::make_unique<DeclContextPrinter>();
}

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
    //CommonOptionsParser op(argc, argv);
    //ClangTool Tool(op.getCompilations(), op.getSourcePathList());
    //int result = Tool.run(newFrontendActionFactory<CustomTokenDumperAction>());
    //runToolOnCode(new ASTDumpAction, argv[1]);
    runToolOnCode(new ASTViewAction, argv[1]);
    //rewriter.getEditBuffer(rewriter.getSourceMgr().getMainFileID()).write(errs());
}