local projectName = "HLMMOServer"
add_rules("mode.debug", "mode.release")
add_requires("boost")


target(projectName)
    set_kind("binary")
    set_languages("cxx20")
    set_exceptions("cxx")


    add_packages("boost")
    add_packages("msgpack")
    add_includedirs(".")
    add_includedirs("./include")
    if is_mode("debug") then
        add_defines("DEBUG")
    end
    
    add_files("main.cpp")
    add_files("./src/*.cpp")

    add_links("GameNetworkingSockets")

    -- Add link search directory
    add_linkdirs(".")
