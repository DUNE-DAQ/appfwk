import moo.otypes

from dunedaq.env import get_moo_model_path
moo.otypes.load_types('appfwk/cmd.jsonnet', get_moo_model_path())
moo.otypes.load_types('appfwk/app.jsonnet', get_moo_model_path())
moo.otypes.load_types('rcif/cmd.jsonnet', get_moo_model_path())
moo.otypes.load_types('cmdlib/cmd.jsonnet', get_moo_model_path())

import dunedaq.appfwk.cmd as cmd 
import dunedaq.appfwk.app as app 
import dunedaq.rcif.cmd as rccmd 
import dunedaq.cmdlib.cmd as ccmd 


def mspec(inst, plugin, conn_refs):
    """
    Helper function to create Module Specification objects
    
    :param      inst:    Instance
    :type       inst:    str
    :param      plugin:  Appfwk Module Plugin name
    :type       plugin:  str
    :param      cinfos:  List of dunedaq.iomanager.ConnectionRef objects
    :type       cinfos:  list
    
    :returns:   A constructed ModSpec object
    :rtype:     dunedaq.appfwk.app.ModSpec
    """
    return app.ModSpec(inst=inst, plugin=plugin,
            data=app.ModInit(
                conn_refs=app.ConnectionReferences_t(conn_refs)
                )
            )

def acmd(mods: list):
    """ 
    Helper function to create appfwk's CmdObj of Addressed module commands.
        
    :param      cmdid:  The coommand id
    :type       cmdid:  str
    :param      mods:   List of module name/data structures 
    :type       mods:   list
    
    :returns:   A constructed Command object
    :rtype:     dunedaq.appfwk.cmd.Command
    """
    return cmd.CmdObj(
        modules=cmd.AddressedCmds(
            cmd.AddressedCmd(match=m, data=o)
            for m,o in mods
        )
    )

def mcmd(cmdid: str, mods: list):
    """
    Helper function to create appfwk's Commands addressed to modules.
    
    :param      cmdid:  The coommand id
    :type       cmdid:  str
    :param      mods:   List of module name/data structures 
    :type       mods:   list
    
    :returns:   A constructed Command object
    :rtype:     dunedaq.appfwk.cmd.Command
    """
    return ccmd.Command(
        id=ccmd.CmdId(cmdid),
        data=acmd(mods)
    )

def mrccmd(cmdid, instate, outstate, mods):
    """
    Helper function to create appfwk's Commands addressed to modules.
    
    :param      cmdid:  The coommand id
    :type       cmdid:  str
    :param      instate:  The state before command execution
    :type       instate:  str
    :param      outstate:  The state after command execution
    :type       outstate:  str
    :param      mods:   List of module name/data structures 
    :type       mods:   list
    
    :returns:   A constructed Command object
    :rtype:     dunedaq.rcif.cmd.RCCommand
    """
    return rccmd.RCCommand(
        id=ccmd.CmdId(cmdid),
        entry_state=rccmd.State(instate),
        exit_state=rccmd.State(outstate),
        data=cmd.CmdObj(
            modules=cmd.AddressedCmds(
                cmd.AddressedCmd(match=m, data=o)
                for m,o in mods
            )
        )
    )

