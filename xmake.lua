set_project("Live")
set_version("1.0.0", {build = "%Y%m%d%H%M"})
set_description("Random Notes from Sickbed")
set_objectdir("build/$(plat)/$(arch)/$(mode)")
set_targetdir("bin/$(plat)/$(arch)/$(mode)")


add_rules("mode.debug", "mode.release")

add_cxxflags("-std=c++17") 
add_cxflags("-Wno-error=deprecated-declarations", "-fno-strict-aliasing", "-Wno-error=expansion-to-defined")
add_mxflags("-Wno-error=deprecated-declarations", "-fno-strict-aliasing", "-Wno-error=expansion-to-defined")
set_warnings("all", "error")

if is_plat("windows") then
    add_defines("PT_WIN")
elseif is_plat("linux") then
    add_defines("PT_LNX")
end

if is_mode("debug") then
    add_cxxflags("-g", "-O0") 
elseif is_mode("release") then
    add_cxxflags("-O3")       
    add_ldflags("-s")         
end

add_includedirs("include")
includes("src/cmockery")
includes("src/ini")
includes("example/inih")
includes("example/cmockery")