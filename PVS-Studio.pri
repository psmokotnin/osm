# 2008-2018 (c) OOO "Program Verification Systems"
#
# Version 2

defineReplace(pvs_studio_filter_sources) {
    variable = $$1
    in = $$eval($$variable)
    out =
    for(source, in) {
        contains(source, "^.+\\.(cpp|cpp|cc|cx|cxx|cp|c\\+\\+)$") {
            out += "$${source}"
        }
        contains(source, "^.+\\.c$") {
            out += "$${source}"
        }
    }
    return($${out})
}

isEmpty(pvs_studio.target) {
    pvs_studio.target = pvs_studio
}
isEmpty(pvs_studio.log) {
    contains(pvs_studio.format, "^tasklist$") {
        pvs_studio.log = "$$pvs_studio.target".tasks
    } else {
        pvs_studio.log = "$$pvs_studio.target".log
    }
}

pvs_studio.empty_config = "no"
isEmpty(pvs_studio.cfg) {
    pvs_studio.cfg = "PVS-Studio.cfg"
    pvs_studio.empty_config = ""
}
isEmpty(pvs_studio.cfg_text) {
    pvs_studio.cfg_text = "analysis-mode=4"
} else {
    pvs_studio.empty_config = ""
}

isEmpty(pvs_studio.cxxflags) {
    pvs_studio.cxxflags = $${QMAKE_CXXFLAGS}
}
isEmpty(pvs_studio.cflags) {
    pvs_studio.cflags = $${QMAKE_CFLAGS}
}

for(define, DEFINES) {
    pvs_studio.cxxflags += "-D$$define"
    pvs_studio.cflags += "-D$$define"
}

for(path, INCLUDEPATH) {
    pvs_studio.cxxflags += "-I$$path"
    pvs_studio.cflags += "-I$$path"
}

for(path, QMAKE_INCDIR) {
    pvs_studio.cxxflags += "-I$$path"
    pvs_studio.cflags += "-I$$path"
}

pvs_sources = $$pvs_studio.sources
pvs_sources = $$pvs_studio_filter_sources(pvs_sources)
pvs_plogs =
pvs_targets =

pvs_cfg = "$$pvs_studio.target"_cfg
$${pvs_cfg}.target = "$$pvs_studio.cfg"
isEmpty(pvs_studio.empty_config) {
    $${pvs_cfg}.commands = echo "$$pvs_studio.cfg_text" > "$$pvs_studio.cfg"
} else {
    $${pvs_cfg}.commands = touch "$$pvs_studio.cfg"
}
QMAKE_EXTRA_TARGETS += "$${pvs_cfg}"

for(source, pvs_sources) {
    contains(source, "^/.*$") {
        relpath = $$relative_path("$$source", "$$_PRO_FILE_PWD_")
        !contains(relpath, "^../") {
            source = "$$relpath"
        }
    }
    dir = $$dirname(source)
    isEmpty(dir) {
        dir = "."
    }
    args = $$pvs_studio.args
    dir = $$clean_path("$$OUT_PWD/PVS-Studio/$${dir}")
    log = $$clean_path("$${dir}/$${source}.plog")
    dir = $$dirname(log)
    !contains(source, "^/.*$") {
        source = "$$_PRO_FILE_PWD_/$${source}"
    }
    source = $$clean_path("$$source")
    contains(source, "^.+\\.(cpp|cpp|cc|cx|cxx|cp|c\\+\\+)$") {
        lang_flags=$$pvs_studio.cxxflags $(CXXFLAGS)
    }
    contains(source, "^.+\\.c$") {
        lang_flags=$$pvs_studio.cflags $(CFLAGS)
    }
    !isEmpty(pvs_studio.license) {
        args += --lic-file \'$$pvs_studio.license\'
    }
    args += --cfg \'$$pvs_studio.cfg\'
    args += --output-file \'$${log}\'
    args += --source-file \'$${source}\'
    args += --platform linux64
    args += --preprocessor gcc
    args += --cl-params $${lang_flags} $(DEFINES) -DPVS_STUDIO $(INCPATH)
    target = $${source}_pvs
    $${target}.target = "$$log"
    $${target}.commands = mkdir -p \'$${dir}\' && \
                          rm -f \'$${log}\' && \
                          pvs-studio-analyzer analyze $${args} \'$${source}\'
    $${target}.depends = "$${source}"
    $${target}.depends += "$${pvs_cfg}"
    pvs_plogs += "$${log}"
    pvs_targets += $${target}
    QMAKE_CLEAN += "$${log}"
    QMAKE_EXTRA_TARGETS += "$${target}"
}

pvs_target = $$pvs_studio.target
$${pvs_target}.depends = $${pvs_targets}
isEmpty(pvs_plogs) {
    $${pvs_target}.commands = echo \"\" > "$$pvs_studio.log"
} else {
    commands = cat $${pvs_plogs} > "$$pvs_studio.log"
    !isEmpty(pvs_studio.output) {
        isEmpty(pvs_studio.format) {
            pvs_studio.format = errorfile
        }
    }
    !isEmpty(pvs_studio.format) {
        commands += && mv \'$$pvs_studio.log\' \'$$pvs_studio.log\'.pvs.raw \
                    && plog-converter -t $$pvs_studio.format -o \'$$pvs_studio.log\' \'$$pvs_studio.log\'.pvs.raw
    }
    !isEmpty(pvs_studio.output) {
        commands += && cat \'$$pvs_studio.log\' 1>&2
    }
    $${pvs_target}.commands = $${commands}
}

QMAKE_CLEAN += "$$pvs_studio.log"
QMAKE_EXTRA_TARGETS += "$${pvs_target}"

