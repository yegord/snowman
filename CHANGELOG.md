*In compliance with the [GPL-3.0](https://opensource.org/licenses/GPL-3.0) license: I declare that this version of the program contains my modifications, which can be seen through the usual "git" mechanism.*  


2021-06  
Contributor(s):  
Yegor Derevenets  
>Parse ELF program headers  
>Parse ELF program headers if sections are emptyFixes https://github.com/yegord/snowman/issues/221  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2021-04  
Contributor(s):  
Yegor Derevenets  
Jeinzi  
>Fix regression in struct field naming  
>Fix Qt deprecation regarding endlReplace all occurrences of endl with \n  
>Fix Qt deprecations regarding hex and decReplace with appropriate calls of QTextStream::setIntegerBase  
>Fix Qt deprecations regarding mouse wheel  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2020-11  
Contributor(s):  
Yegor Derevenets  
>Get rid of deprecated Qt's qSort()  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2020-05  
Contributor(s):  
Yegor Derevenets  
>Remove redundant moves  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2019-09  
Contributor(s):  
Yegor Derevenets  
>Change passing by value to passing by ref where appropriate  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2019-05  
Contributor(s):  
Yegor Derevenets  
>Do not link nocode with Qt5WidgetsFixes https://github.com/yegord/snowman/issues/198  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2019-04  
Contributor(s):  
Yegor Derevenets  
>Fix CxxView::getIntegerAtCursor for hexhttps://github.com/yegord/snowman/issues/196#issuecomment-485400325  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2018-06  
Contributor(s):  
Yegor Derevenets  
>Fix classification of LikeC types  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2018-05  
Contributor(s):  
s0i37  
>added just function decompiling mode  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2018-01  
Contributor(s):  
Yegor Derevenets  
Alexey Svirchevsky  
>Be more optimistic about C stringsFixes https://github.com/yegord/snowman/issues/151  
>parser for DOS extender + LE executables  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2017-11  
Contributor(s):  
Yegor Derevenets  
>Update IdaByteSource to support IDA 7.0  
>Update FindIDA.cmake to support IDA 7.0  
>Fix IDA 7 crash on exit  
>Require CMake 3.7 only  
>Update the rest of the plugin to support IDA 7.0  
>Update minimum required CMake version  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2017-10  
Contributor(s):  
Yegor Derevenets  
7anner  
Alexey Svirchevsky  
>Removed unnecessary FALLTHROUGH  
>fix bug reporting link in the About Snowman dialoglooks like it used to be an email  
>removed duplicate byte order conversion  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2017-08  
Contributor(s):  
Yegor Derevenets  
>Added highlighting customization via style sheets  
>Added highlighting customization via style sheetshttps://github.com/yegord/snowman/issues/94  
>More const  
>Let the user specify a Qt style sheet fileFixes https://github.com/yegord/snowman/issues/94  
>Added missing const  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2017-07  
Contributor(s):  
Marco Milanese  
>Add sanitizers supportAdd an option to enable sanitizers. For now, only address, leak andundefined are enabled.  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2017-04  
Contributor(s):  
Yegor Derevenets  
>Fix detection of substitutions of complex expressions  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2017-03  
Contributor(s):  
Luca Weiss  
>Fix capstone link in About  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2017-02  
Contributor(s):  
Yegor Derevenets  
>Image base symbol should have a value  
>Got rid of 0x prefixes in addresses  
>recognizeBoundsCheck: Unconditional jump is not a bounds checkThis fixes https://github.com/yegord/snowman/issues/107  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2017-01  
Contributor(s):  
Yegor Derevenets  
Fabian Freyer  
>Move RangeNode and RangeTree logic to coreIt contains no range-specific code and could ideally be used from core.  
>printRanges: allocate CxxDocument on the stack  
>More explicit constructors in nc::gui  
>More code movement* Move Range{Node, Tree, TreeBuilder} from nc::core to nc:: and into  src/nc/common directory.* Move CxxDocument from nc::core to nc::core::likec  
>Set minimum required CMake version to 2.8.11Older CMake does not support CMP0020.Fixes https://github.com/yegord/snowman/issues/105  
>Moved GUI-Unspecific stuff from CxxDocumentSeparated out all non-GUI-Relevant functions fromnc::gui::CxxDocument into nc::core::CxxDocument. Thisallows using the CxxDocument range features withoutthe GUI.nc::gui::CxxDocument now inherits from QTextDocumentas well as from nc::core::CxxDocument.  
>printRanges: proper address formatting  
>Do not warn about absence of instructions  
>nocode: use a std::shared_ptr for contextsThis is consistent with the usage in nc::gui.  
>printRanges: Coding style  
>Don't use shared_pointer for non-GUI CxxDocument  
>Output source code to instruction mappingThis is used by e.g. radare2, which until now does a very uglyhack [1] to extract this information.[1] https://github.com/radare/radare2-extras/blob/master/r2snowman/snowman-r2.patch  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2016-09  
Contributor(s):  
Yegor Derevenets  
>Fix plug-in build with IDA 6.95  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2016-06  
Contributor(s):  
Duncan Ogilvie  
>workaround for a behavior change in Qt > 5.4https://bugreports.qt.io/browse/QTBUG-47042  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2016-05  
Contributor(s):  
Jeff Muizelaar  
>Add support for base relocsThis makes the disassembler skip over relocations and parsesthe base relocation section of PE files.  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2016-04  
Contributor(s):  
Yegor Derevenets  
>Compute abstract values starting from "can be anything"Fixes https://github.com/yegord/snowman/issues/82Fixes https://github.com/yegord/snowman/issues/83  
>Get rid of unused variable warning on GCC  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2016-02  
Contributor(s):  
Yegor Derevenets  
Karl Vogel  
>Add entry point to image parsing  
>Allow platform to specify the integer size.  
>Small fix in the doxygen comment  
>Added missing explicit  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2016-01  
Contributor(s):  
Yegor Derevenets  
Duncan Ogilvie  
>fixed a bugs with syntax highlightingThe operator `-` was not recognized because the regex contained `*-+` which matches a character in the range of `*` to `+`. This was updated to `*\\-+` which has the correct behavior. I also added `(u)intX_t` as a keyword so everything looks a little nicer.  
>Undo byte swapping done by IDA  
>Fix detection of big endian ARM in IDA plugin  
>Added missing #include  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2015-11  
Contributor(s):  
Yegor Derevenets  
>Qt 5.5 adds new log message type  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2015-09  
Contributor(s):  
Yegor Derevenets  
>LikeC: static_cast, reinterpret_cast  
>Removed xxx::USER constants  
>Added missing includeFixes https://github.com/yegord/snowman/issues/52  
>Added missing byte order conversion in PE parserFixes https://github.com/yegord/snowman/issues/46  
>Got rid of tree reference in likec::TreeNode  
>Workaround MSVC 2010 bug  
>Added likec::Simplifier  
>Moved simplification to a subdirectory  
>Fix CMake CMP0054 warnings  
>Minor code cleanup  
>Added likec::UndeclaredIdentifier  
>Better reconstruction of expressions  
>Fix failing assertion at DefinitionGeneration:512Fixes https://github.com/yegord/snowman/issues/32  
>Added likec::TypeCalculator  
>Got rid of ir::Choice and ir::Term::KILLFixes https://github.com/yegord/snowman/issues/55  
>MSVC 2015 supports noexcept  
>TreePrinter  
>Moved DefinitionGenerator::isDominating to Utils.{cpp,h}  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2015-08  
Contributor(s):  
Yegor Derevenets  
Jeff Muizelaar  
>More LikeC rewrites for mathematical identities  
>More optimistic type reconstruction for ZERO_EXTEND  
>Generate function names only for constants pointing to the code  
>Stricter requirements for guessed ASCII strings  
>Fix warnings about unused variables under GCC 5  
>More sensible generation of braces in Typecast operator  
>Make results of signed/zero extend signed/unsignedFixes https://github.com/yegord/snowman/issues/40  
>Add PE export parsing  
>More careful reconstruction of expressions  
>Rewriting to array subscript operator  
>Simplification of boolean expressions  
>No braces for array subscript inside a typecast  
>Fix regressions in expression reconstruction  
>More error checking in parsing PE exports  
>Fix vector subscript out of range error in Debug MSVC builds  
>Propagating pointee types for ADD in SUB in both directions  
>Space is actually a printable symbol  
>Better simplification of pointer arithmetic and identities  
>Rewrite: a + -1 -> a - 1  
>Added ARRAY_SUBSCRIPT binary operator  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2015-07  
Contributor(s):  
Yegor Derevenets  
Johannes Obermayr  
>Indentation fix  
>Added missing override, fixed a Clang warning about it  
>Fixes in the doxygen comments  
>Eliminate dead assignments done by calls  
>Added missing switch case in Region::print  
>Got rid of dflow::ExecutionContext  
>Mach-O parser  
>Support for conditional return  
>Make clang and gcc5 compiler detection working.  
>Removed unnecessary 'virtual'  
>Added Platform class  
>Code cleanup  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2015-06  
Contributor(s):  
Yegor Derevenets  
Mathieu René  
>Fixed computation of whether a value is a return address  
>Switched back from capstone to libudis86Fixes https://github.com/yegord/snowman/issues/11  
>Using nullptr instead of NULL everywhereFixes https://github.com/yegord/snowman/issues/15  
>Prevent 3rd-party modules from installing anything  
>Fix IDA MAC build.Includes fcntl.h before the ida workaround header, so that the systemheader is not confused by the conflicting pid_t preprocessor defines.Define USE_DANGEROUS_FUNCTIONS so IDA accepts we use some string functionsLink with libida(64).dylib when producing the IDA pluginName the ida plugin using the mac convention (pmc/pmc64)Treat clang as gcc when looking for static libraries  
>Support building with Qt5Fixes https://github.com/yegord/snowman/issues/18  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2015-05  
Contributor(s):  
Yegor Derevenets  
Mr. eXoDia  
>Fix crashes related to search and go to line widgetshttps://github.com/yegord/snowman/pull/4  
>Updated references to licenses file  
>Added workaround for QTBUG-22829  
>Updated the license reported by the executables  
>Switched from libudis86 to capstone  
>Workaround a libc++ bughttps://github.com/yegord/snowman/pull/3  
>Got rid of external demangler  
>fixed ugly margins  
>Added license comments  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2014-10  
Contributor(s):  
Yegor Derevenets  
>It seems, IDA's demangler can return negative values on failureFor example, it was reported to return -6.  
>More credits to Capstone  
>Mention ARM in about dialog  
>Removed unused Functions::getFunctionsAtAddress()  
>Replaced Conversions.* with StringToInt.*  
>Fixed a bug in escaping  
>Pass FORM_QWIDGET to open_tform()  
>Switched from Kinds.h to Subclass.h  
>Supporting 32-bit build  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2014-09  
Contributor(s):  
Yegor Derevenets  
>More checks  
>All name generation is now done in cgen::NameGenerator  
>Function names in declarations and definitions are refactorable  
>Rehighlighting references after refactoring  
>Return value reconstruction works again on test 014_calls  
>X without modifiers is no longer a safe shortcut for CxxView  
>Generate friendlier code for intrinsics  
>Started making GUI fast  
>Added TODOs  
>Use reasonable defaults when settings are not there  
>Implemented "Go to Definition"  
>Mouse tracking is not needed for receiving ToolTip events  
>Taking into account PE optional header magic  
>Tooltip shows function definition if possible  
>Avoiding name clash with QWIdget::setFont()  
>Support for ARM in PeParser  
>Font setting works again  
>PE import lookup can actually be ignored  
>Removal of characters also works  
>Everything works again  
>Function declarations and definitions are now refactorable  
>Started to implement parsing of PE imports  
>Cleanup  
>IDA plugin: imports are added as relocations  
>Font in TreeView widgets is also changable  
>Blocking signals is no longer necessary  
>Ultra fast highlighting of instructions  
>Fast generation of tooltips for long declarations  
>Formulae was renamed to IR a long time ago  
>Simplifications of CxxDocument  
>Symbols can have undefined values  
>Labels are now refactorable  
>Range template class objects are now totally ordered  
>Renaming dialog  
>Prevent creating too many extra selections  
>Code simplification  
>Tooltips are working again  
>Parsing imports  
>Removed Return statement completely  
>Make highlighting more visible  
>Fixed doxygen warning  
>Got rid of BufferByteSource and ZeroByteSource  
>Avoid hardcoding of colors  
>Builds under MSVC 2010  
>Support ARM in IDA plug-in  
>Refactored CxxView::highlightReferences()  
>Code cleanup  
>Dataflow analysis is performed on a CFG, not on a function  
>Removed unused header  
>Highlighting when clicking on a variable declaration  
>Give some meaningful names to ordinal-value imports  
>Not creating NameGenerator every time  
>Changing one identifier changes them all  
>Removed TextRange.h  
>Exact uses  
>Cleaned up inlining code  
>Suppress warnings from Boost  
>Further cleanup of GUI models  
>Variable declarations are now refactorable  
>Small fix in doxygen documentation  
>Ultra fast highlighting in TextView  
>Refactoring partially works  
>Zero-extending PE sections  
>Show tooltips with refactoring results  
>Fixed doxygen warnings  
>Simpler InstructionsModel  
>Added hotkeys for navigation and renaming in CxxView  
>Go to definition now works  
>Importing capstone as a subproject  
>Go to declaration -> Go to definition  
>RETURN_ADDRESS intrinsic  
>Rebranding  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2014-08  
Contributor(s):  
Yegor Derevenets  
>Per-domain byte order  
>Font selection in TextView  
>Added log levels  
>Added support for missing statement kinds in InspectorModel  
>Merged types::FunctionAnalyzer into TypeAnalyzer  
>Access type is no longer stored in terms  
>Got rid of *INSTRUCTION_ADDRESS intrinsics  
>PeParser fills in symbol's section  
>Added Halt IR statement  
>Initial skeleton for the ARM architecture  
>IDA is no longer Pro  
>More ArmInstructionAnalyzer code  
>Got rid of ncWarning  
>Removed unused headers  
>Term::callOnChildren  
>Honouring ImageBase in the PE parser  
>Removing empty if branches  
>Return value locations described by memory locations, not terms  
>Code cleanup  
>Added capstone version aa791a2 to the source tree  
>Removed Visitor class  
>RememberReachingDefinitions statement  
>Added Statement::isTerminator() method  
>Removed unused argument  
>Structural analysis is more careful when reducing head nodes  
>Less calls to Term::source()  
>Bounds check recognition can now cope with boolean negations  
>Reader::readInt was incorrect for big endian byte order  
>Symbol type names are defined in one place now  
>Got rid of QObject::tr() calls  
>Refactored DataflowAnalyzer  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2014-07  
Contributor(s):  
Yegor Derevenets  
>Not deleting underscores from names of functions and variables  
>ElfParser: relocations, byte order  
>Generating names of global variables from relocations  
>Removed operand classes  
>Proper handling of bss sections  
>Plug-in builds and works again  
>Generating initial values for global variables  
>Parsing .dynsym symbols in ELF too  
>CMake flag for enabling profiling  
>Fixed compiler error under MSVC  
>Got rid of image::Sections class  
>Got rid of mnemonics  
>Removed unused CommentStatement  
>Less whitespaces at line ends  
>Support for initial values in LikeC variable declarations  
>Cleanup  
>InstructionAnalyzer and InstructionDisassembler can have state  
>Variable names are not mangled  
>Got rid of Image::Symbols class  
>Fixed generation of variable names  
>Sorting in SectionsView and SymbolsView  
>Cleanup in the PeParser  
>Fixed handling of RIP-relative addressingThe displacement is applied to the address of the _next_ instruction.  
>Got rid of IntelInstruction::name()  
>Showing licensing information in the decompiler  
>More separators in context menus  
>Byte order handling in PeParser  
>Fixed the size of created integer constants  
>Merged Disassembler and InstructionDisassembler classes  
>Cleaner generation of variable's initial value  
>Targeting Windows XP  
>Added missing header: nc/config.h does not include boost/config.hpp  
>Removed unused code from core::arch::Registers  
>Got rid of image::Relocations class  
>core::arch::irgen -> core::irgen  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2014-06  
Contributor(s):  
Yegor Derevenets  
>Pointers to types of size > 1 should not be looked for C strings  
>Functions must be drawn in separate boxes  
>Made branding information customizable  
>Set allocated flag for sections imported from IDA  
>Fixed C4103 warning under MSVC  
>Jump to selected symbol address context menu item  
>Build libiberty and undname unconditionally  
>Command-line demangler  
>More descriptive name of git commit  
>Proper escaping of double quotes in C strings  
>Replaced CxxFiltDemangler with generic ExternalDemangler  
>Added install targets  
>Generating function names instead of constants  
>Workaround MSVC2010 bugs  
>Trim function entries containing no-ops in the beginning  
>Removed unnecessary typecast  
>Added BundledDemangler, removed Gnu and Msvc demanglers  
>More optimistic C string reconstruction  
>Do not use __DATE__ to keep builds reproducible  
>SymbolsModel, SymbolsView  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2014-05  
Contributor(s):  
Yegor Derevenets  
>Setting basic block pointer in statements to NULL on removal  
>Moved ir::*Operator::apply to DataflowAnalyzer  
>Added missing #includes of nc/config.h  
>Moved DCMAKE_EXPORT_COMPILE_COMMANDS=YES to CMakeLists.txt  
>Added TODO  
>Fixed a crash  
>Disabled -Wmaybe-initialized warnings on GCC  
>Do not reduce regions including parent's entry...unless it is subregion's entry  
>Simpler check whether a variable is local  
>Printing address and basic block of a cflow::Graph's nodes  
>Do not generate code for argument and return value statements  
>ilist code cleanup  
>Expression reconstruction: first attempt  
>Removed invalid assertion  
>Fixed texts of warnings  
>Removing disappeared terms from the dataflow information  
>Print revision in command-line decompiler too  
>Renamed project from NOCODE to Decompiler  
>Bugfixes  
>Bugfix in Program::createBasicBlockIt no longer crashes if the result of the split is empty. This canhappen, e.g., when you want to split a basic block, whose successoraddress is bigger then the address of the end of its instruction. Anexample of such a basic block is the one which is created whenconverting ret + nop to IR.  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2014-04  
Contributor(s):  
Yegor Derevenets  
>Unused include  
>More of const-correctness  
>No need to recreate hooks manager  
>Resolved FIXME in TypeAnalyzer::joinArgumentTypes  
>No longer crashes  
>Fixed copying instrumentation statements in wrong order in EntryHook  
>Fixes in doxygen documentation  
>Instrumentation of entries, calls, returns  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2014-03  
Contributor(s):  
Yegor Derevenets  
>New argument reconstruction algorithm  
>Got rid of CensusVisitor and visitors in IR in general  
>Introduced intrusive lists  
>Intrusive list everywhere  
>No CensusVisitor used in type reconstruction code  
>Typedef for ilist<Statement>  
>Replaced ir::Kill by more generic ir::Touch  
>More TODOs  
>More refactoring  
>Added forgotten member initialization  
>Fixed: type reconstruction did not terminate  
>Got rid of misc::TermToFunction  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2014-01  
Contributor(s):  
Yegor Derevenets  
>Handling arguments not aligned to corresponding variables  
>Renamed Image to Sections, Module to Image  
>std::uintptr_t -> uintptr_t  
>Fixed build errors in plug-in  
>DeclarationGenerator can generate declarations out of signatures  
>Fixed translation of LOOP instruction to IR  
>Fixed FIXMEs  
>Assertion violation fixed  
>Added support for symbols  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2013-12  
Contributor(s):  
Yegor Derevenets  
>Interprocedural type reconstruction  
>Added scope property to variables  
>Grammar fixes  
>Cleaner implementation of joinVariableTypes()  
>Simplified foreach implementation  
>One census less  
>Fixes in doxygen documentation  
>Reconstruction of global variables  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2013-11  
Contributor(s):  
Yegor Derevenets  
>Added CancellationToken::poll()  
>Fixed computation of dominators  
>Small cleanup  
>Doxygen errors fixed  
>Fixed a bug in ReachingDefinitions::filterOut()  
>Renamed FunctionSignature to Signature  
>Memoization of isIntermediate(), isSingleAssignment() results  
>Computing dominator sets  
>Renamed usage to liveness  
>Minor cleanup  
>Merged GenericCallingConvention with CallingConvention  
>Allow undefined signatures  
>Removed fixpointReached and computation of uses from dflow analysis  
>Made reconstruction of signatures a separate step  
>TODO added  
>Renamed FunctionAnalyzer to EnterHook  
>More of arguments reconstruction  
>Proof-of-concept arguments reconstruction  
>Renamed Term::assignee() to Term::source()  
>Renamed CallsData to Hooks  
>Moved getReturns() from CallsData to Function  
>Renamed CallingConvention to Convention  
>Moved ir::CommentText to ir::misc  
>Merged GenericEntryHook with EntryHook  
>cconv -> calling  
>Got rid of CallingConventionDetector  
>New elimination of intermediate variables works  
>Renamed {Call,Return}Analyzer to {Call,Return}Hook  
>Merged CallHook and GenericCallHook, removed *Offset types  
>New isIntermediate()  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2013-10  
Contributor(s):  
Yegor Derevenets  
>Verbose mode in command-line decompiler  
>ArchitectureRepository  
>One more rewrite rule for pointer arithmetics  
>Missing headers  
>Updated --help documentation for nocode  
>Enabled -Wall for Clang  
>Type reconstruction adapted to the new dataflow  
>Cleanup of the command-line decompiler's code  
>There are actually no variable length arrays in C++  
>Minor cleanup  
>Cleaner AbstractValue, fixed unused parameter warnings  
>Workaround buggy decltype of MSVC 2010  
>Distinct names for detail namespaces  
>Fixed compilation error in IDA plug-in  
>More assertions  
>No narrowing conversions  
>Cleaner creation of local variables  
>Renamed pairs to chunks in ReachingDefinitions  
>Another workaround for MSVC  
>Added core::Driver class  
>Introduced operand caches, made Architecture immutable  
>Cleaner ParserRepository  
>List of calling conventions in core::arch::Architecture  
>Cleaner variables reconstruction code  
>Less narrowing conversions  
>Renamed UniversalAnalyzer to MasterAnalyzer  
>Generating pointer arithmetics for variable accesses  
>One copy less  
>Replaced Exact class with enum, in order to avoid link errors  
>Got rid of inlining in command-line decompiler  
>Updated getFirstCopy to new dataflow  
>Renamed ir::calls to ir::cconv  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2013-09  
Contributor(s):  
Yegor Derevenets  
>Deleting moved terms from reaching definitions  
>Removed unnecessary core:: namespace specifiers  
>No-truncate constructors of SizedValue and AbstractValue  
>Merge abstract values for IR operators too  
>AbstractValue  
>Filtering out definitions that changed their location  
>Further refactoring of dataflow  
>Full merge of reaching values  
>Cleanup in terms and statements  
>Fix in computing stack offset  
>Taking into account ByteOrder in image::Reader  
>Refactoring of core::image  
>TODO added  
>Byte order  
>No more census in dataflow analysis  
>More documentation in common/Range.h  
>No default values for SizedValue's constructor arguments  
>More refactoring  
>More refactoring, removed BitRange operand  
>Bug fix in operator== for abstract values  
>Variable reconstruction, builds again  
>Rewrite ~ to ! in obvious cases  
>Does not crash on 002 example  
>Strict checking of sizes in IR operators  
>Fixed nc::bitMask()  
>Memory allocation/deallocation optimizations  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 


2013-08  
Contributor(s):  
Yegor Derevenets  
>Bit-precise killDefinitions()  
>Cleaned up dflow::Dataflow class  
>Unused code  
>Rewrote ReachingDefinitions class  
- - - - - - - - - - - - - - - - - - - - - - - - - - - 

