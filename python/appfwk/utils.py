import moo.otypes

from dunedaq.env import get_moo_model_path
moo.otypes.load_types('appfwk-cmd-schema.jsonnet', get_moo_model_path())

import dunedaq.appfwk.cmd as cmd 

def mspec(inst, plugin, qinfos):
    """
    Helper function to create Module Specification objects
    
    :param      inst:    Instance
    :type       inst:    str
    :param      plugin:  Appfwk Module Plugin name
    :type       plugin:  str
    :param      qinfos:  List of dunedaq.appfwk.QueueInfo objects
    :type       qinfos:  list
    
    :returns:   A constructed ModSpec object
    :rtype:     dunedaq.appfwk.cmd.ModSpec
    """
    return cmd.ModSpec(inst=inst, plugin=plugin,
            data=cmd.ModInit(
                qinfos=cmd.QueueInfos(qinfos)
                )
            )

def mcmd(cmdid, mods):
    """
    Helper function to create appfwk's Commands addressed to modules.
    
    :param      cmdid:  The coommand id
    :type       cmdid:  str
    :param      mods:   List of module name/data structures 
    :type       mods:   list
    
    :returns:   A constructed Command object
    :rtype:     dunedaq.appfwk.cmd.Command
    """
    return cmd.Command(
        id=cmd.CmdId(cmdid),
        data=cmd.CmdObj(
            modules=cmd.AddressedCmds(
                cmd.AddressedCmd(match=m, data=o)
                for m,o in mods
            )
        )
    )
