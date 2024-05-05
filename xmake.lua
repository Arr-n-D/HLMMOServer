local projectName = "HLMMOServer"

target(projectName)
    set_kind("binary")
    set_languages("cxx20")
    set_exceptions("cxx")

    add_includedirs(".")
    add_includedirs("./include")
    
    add_files("main.cpp")
    add_files("./src/*.cpp")

    add_links("GameNetworkingSockets")

    -- Add link search directory
    add_linkdirs(".")