import moo.otypes

from dunedaq.env import get_moo_model_path
moo.otypes.load_types('appfwk-cmd-schema.jsonnet', get_moo_model_path())

import dunedaq.appfwk.cmd as cmd # AddressedCmd, 

def mspec(inst, plugin, qinfos):
    return cmd.ModSpec(inst=inst, plugin=plugin,
            data=cmd.ModInit(
                qinfos=cmd.QueueInfos(qinfos)
                )
            )

def mcmd(cmdid, mods):
    return cmd.Command(
        id=cmd.CmdId(cmdid),
        data=cmd.CmdObj(
            modules=cmd.AddressedCmds(
                cmd.AddressedCmd(match=m, data=o)
                for m,o in mods
            )
        )
    )
