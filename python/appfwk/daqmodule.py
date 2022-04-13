

class DAQModule:
    """An individual DAQModule within an application, along with its
       configuration object
    """

    def __init__(self, plugin, conf=None, extra_commands=None, name="__module"):
        self.plugin=plugin
        self.name=name
        self.conf=conf
        self.extra_commands=extra_commands if extra_commands else dict()

    def __repr__(self):
        return f"{self.name} module(plugin={self.plugin}, conf={self.conf}, connections={self.connections})"

    def __rich_repr__(self):
        yield "plugin", self.plugin
        yield "name", self.name
        yield "conf", self.conf
        yield "extra_commands", self.extra_commands
