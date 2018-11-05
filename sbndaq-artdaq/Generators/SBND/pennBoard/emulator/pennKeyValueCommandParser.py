import string

from pennCommandParser import PennCommandParser

class PennKeyValueCommandParser(PennCommandParser):
    """
    PennCommandParser - command parser for PENN emulator
    """

    def __init__(self):

        super(PennKeyValueCommandParser, self).__init__()
        
        # Define dictionary of legal commands and assoicated methods
        self.commands = { 'START' : self.start_cmd,
                          'STOP'  : self.stop_cmd,
                          'PING'  : self.ping_cmd,
                          'SET'   : self.set_cmd,
                          'GET'   : self.get_cmd,
                        }
 
    def parse_request(self, cmd_data):
        """
        Command parser - takes a line of space-separated data and parses it
        into a command and associated parameter arguments
        """

        # Split data into space separated list
        cmd_words = string.split(cmd_data)

        #print ">>>", cmd_words

        req_command = cmd_words[0].upper()

        # Test if first word, i.e. the command word, is a legal command
        if req_command in self.commands:

            # If so, split remaining words into key-value pairs separated by '=''
            try:
                args = dict(item.split("=") for item in cmd_words[1:])

                # Pass arguments to command method
                (cmd_ok, reply) = self.commands[req_command](args)

            except ValueError:
                cmd_ok = False
                reply = "Illegal argument format specified"

        else:
            cmd_ok = False
            reply = "Unrecognised command"

        # Format response depending on success of command
        if cmd_ok:
            response = "ACK {} {}\n".format(req_command, reply)
        else:
            response = "NACK {} error=\"{}\"\n".format(req_command, reply)
        print "  >>> ", response,
        return response

    def start_cmd(self, args):
        """
        Handle a START command to transmit simulated PENN data fragments over UDP
        """

        cmd_ok = True
        reply=""

        print "Got START command", args

        # Loop over legal parameters for the data sender; if they exist in
        # the current parameter list, call the appropriate set_ method on the sender.
        # Trap any exceptions raised by the set_ methods trying to convert
        # an argument value to the approriate type
        try:
            for param in ['host', 'port', 'microslice_size_rollover', 'rate', 'millislices', 'microslices', 'payload_mode', 'trigger_mode', 'fragment_microslice_at_ticks', 'repeat_microslices', 'debug_partial_recv']:
                if param in self.params:
                    getattr(self.sender, 'set_' + param)(self.params[param])

        except ValueError as e:
            print "Error parsing argument to START command:", e
            cmd_ok = False

        except AttributeError as e:
            print "Attempted to set illegal parameter for data sender:", e
            cmd_ok = False

        else:
            (cmd_ok, reply) = self.sender.run()

        return cmd_ok, reply

    def stop_cmd(self, args):

        cmd_ok = True
        reply = ""
        print "Got STOP command", args

        (cmd_ok, reply) = self.sender.stop()
        
        return cmd_ok, reply

    def ping_cmd(self, args):

        cmd_ok = True
        reply = ""
        print "Got PING command", args

        return cmd_ok, reply

    def set_cmd(self, args):

        cmd_ok = True
        reply = ""
        print "Get SET command", args

        for required_arg in ['param', 'value', 'type']:
            if not required_arg in args:
                reply = "Required {} argument missing".format(required_arg)
                cmd_ok = False

        if cmd_ok:
            try:
                self.params[args['param']] = eval(args['type'])(args['value'])

            except NameError as e:
                reply = "Failed to set parameter {} of type {} to value {} : {}".format(args['param'], args['type'], args['value'], e)
                cmd_ok = False

        return cmd_ok, reply

    def get_cmd(self, args):

        cmd_ok = True
        reply = ""
        print "Got GET command", args

        if 'param' in args:
            if args['param'] in self.params:
                reply = "param={} value={}".format(args['param'], self.params[args['param']])
            else:
                reply = "No such parameter {}".format(args['param'])
                cmd_ok = False
        else:
            reply = "No param argument specified"
            cmd_ok = False

        return cmd_ok, reply
