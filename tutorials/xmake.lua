rule("tutorial_base")
    add_deps("win.sdk.application")

    after_load(function(target)
        target:add("deps", "hgex", "hgexhelpers")
        target:set("group", "tutorials")
        target:set("rundir", "$(projectdir)/tutorials/precompiled")
    end)

for _, dir in ipairs(os.filedirs("tutorial*")) do
    target(path.basename(dir))
        add_rules("tutorial_base")
        add_files(path.join(dir, "*.cpp"))
end