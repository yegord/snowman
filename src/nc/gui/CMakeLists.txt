set(MOC_HEADERS
    Activity.h
    Command.h
    CommandQueue.h
    CxxDocument.h
    CxxView.h
    Decompilation.h
    Decompile.h
    DecompileAll.h
    DeleteInstructions.h
    Disassemble.h
    Disassembly.h
    DisassemblyDialog.h
    GotoLineWidget.h
    InspectorModel.h
    InspectorView.h
    InstructionsModel.h
    InstructionsView.h
    LogManager.h
    LogView.h
    MainWindow.h
    Project.h
    SearchWidget.h
    SectionsModel.h
    SectionsView.h
    SymbolsModel.h
    SymbolsView.h
    TextEditSearcher.h
    TextView.h
    TreeView.h
    TreeViewSearcher.h
)

set(SOURCES
    Activity.cpp
    Colors.h
    Command.cpp
    CommandQueue.cpp
    CppSyntaxHighlighter.cpp
    CxxDocument.cpp
    CxxView.cpp
    Decompilation.cpp
    Decompile.cpp
    DecompileAll.cpp
    DeleteInstructions.cpp
    Disassemble.cpp
    Disassembly.cpp
    DisassemblyDialog.cpp
    GotoLineWidget.cpp
    InspectorItem.cpp
    InspectorModel.cpp
    InspectorView.cpp
    InstructionsModel.cpp
    InstructionsView.cpp
    LogManager.cpp
    LogView.cpp
    MainWindow.cpp
    ParentTracker.h
    Project.cpp
    RangeNode.h
    RangeTree.cpp
    RangeTree.h
    RangeTreeBuilder.h
    SearchWidget.cpp
    SectionsModel.cpp
    SectionsView.cpp
    SymbolsModel.cpp
    SymbolsView.cpp
    TextEditSearcher.cpp
    TextView.cpp
    TreeView.cpp
    TreeViewSearcher.cpp
)

qt4_wrap_cpp(SOURCES ${MOC_HEADERS} OPTIONS -DQ_MOC_RUN)

add_library(nc-gui ${SOURCES})
target_link_libraries(nc-gui nc-core ${Boost_LIBRARIES} ${QT_LIBRARIES})

# vim:set et sts=4 sw=4 nospell:
