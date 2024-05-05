local projectName = "HLMMOServer"

target(projectName)
    set_kind("binary")
    set_languages("cxx20")
    set_exceptions("cxx")

    add_files("main.cpp")
    add_includedirs(".")
    add_includedirs("./include")