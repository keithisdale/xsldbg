XSLDBG_VERSION="4.5.1"
DEFINES+=XSLDBG_VERSION=\\\"$$XSLDBG_VERSION\\\"

!xsldbg_GUI {
    # Documentation is now installed via ../docs/en/en.pro
    unix {
            DOCS_ROOT="$$[QT_INSTALL_DOCS]/xsldbg"
            DEFINES+= DOCS_PATH="\"\\\"$$DOCS_ROOT/en\\\"\""
    }

    win32{
            # DEFINES+= DOCS_PATH="\"\\\"$$INSTALL_PREFIX\\docs\\en\\\"\""
    }
}

