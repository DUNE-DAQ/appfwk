// hand written helpers to make object that match appfwk-cmd-schema.
{
    // Make a queue spec.
    qspec(inst, kind, capacity) :: {
        inst:inst, kind:kind, capacity:capacity,
    },

    // queue direction attribute, use this to catch spelling errors.
    qdir: { unknown:"unknown", input: "input", output: "output" },

    // Build a quinfo to provide to the module part of an init command object.
    qinfo(name, inst, dir=self.qdir.unknown) :: {
        name:name, inst:inst, dir:dir
    },

    // Make a module spec.  Data is any custom data (beyond queue
    // locating info) needed by the module's "init" handler.  Most
    // modules have none as their customization is handled by the
    // "conf" command.
    mspec(inst, plugin, qinfos=[], modini={}) :: {
        local qis = if std.type(qinfos) == "array" then qinfos else [qinfos],
        inst:inst, plugin:plugin, data: {qinfos:qis} + modini,
    },

    // Construct a cmdobj for an "init" command from arrays of
    // qspecs and mspecs.  
    init(qspecs, mspecs) :: {
        id: "init",
        data: {
            queues: qspecs,
            modules: mspecs
        },
    },

    // Make a matched command (or module command or mcmd).  This
    // makes an object of AddressedCmd type.  The match is, in
    // general, a regex against available target (module) inst
    // names.  Of course, this match may be a literal inst name.
    // The cmdobj is the data structure to dispatch to all matched
    // instances.  It is likely created with a make.helper from
    // here or from the the schema for the particular thing (ie,
    // particular module implementation schema).  The empty string
    // is interpreted to be equivalent to the regex '.*'.
    //
    // A fully empty mcmd(), when associated with a commnd ID,
    // acts as a "broadcast signal" command (no payload, send to
    // all).
    mcmd(match="", cmdobj={}) :: { match:match, data:cmdobj },
    
    local defaddr = [self.mcmd()],

    // Make a non-init command object.  The addressed should be an
    // array of matched commands (mcmd() output).
    cmd(id, addressed = defaddr) :: { id:id, data:{modules:addressed} },

    // A "conf" command with an array of matched commands.  See mcmd() comment.
    conf(mcmds) :: self.cmd("conf", mcmds),

    // A "start" command sent to all modules
    start(runnum) :: self.cmd("start", [self.mcmd(cmdobj={run:runnum})]),

    // A "stop" command, by default sent to all modules
    stop(addr=defaddr) :: self.cmd("stop", addr),

    // A "scrap" command, by default sent to all modules
    scrap(addr=defaddr) :: self.cmd("scrap", addr),

    // A "fini" command, by default sent to all modules
    fini(addr=defaddr) :: self.cmd("fini", addr),
}
