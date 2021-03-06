include(../xsldbg.pri)

TEMPLATE = subdirs

testdocuments.files =\
     $$files(*.x*l) \
     $$files(*.dtd) \
    $$files(*.txt)

testdocuments.path = $$DOCS_ROOT/tests
INSTALLS += testdocuments

testsubdocuments.files =\
     $$files(subdir\*.x*l)
testsubdocuments.path = $$DOCS_ROOT/tests/subdir
INSTALLS += testsubdocuments
