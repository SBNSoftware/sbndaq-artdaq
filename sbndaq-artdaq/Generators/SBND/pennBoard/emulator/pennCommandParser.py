from pennDataSender import *

class PennCommandParser(object):
    """
    PennCommandParser - command parser for PENN emulator
    """

    def __init__(self):

        # Define empty dictionary of legal commands and associated methods
        self.commands = { }

        # Define empty dictionary of parameters to be set/get
        self.params = {}
        
        # Create a data sender to use
        self.sender = PennDataSender(use_tcp=True) 
