import string
from xml.etree import ElementTree

from pennCommandParser import PennCommandParser

class PennXmlCommandParser(PennCommandParser):
    
    def __init__(self):
        
        super(PennXmlCommandParser, self).__init__()

        # Define dictionary of legal commands and associated methods
        self.commands = { 
            'SoftReset'     : self.softReset_cmd,
            'HardReset'     : self.hardReset_cmd,
            #'ReadXmlFile'   : self.readXmlFile_cmd,
            'SetRunState'   : self.setRunState_cmd,
                        }
        
        # Empty dictionary to contain any configuration parameters set by command or 
        # loading file
        self.params = {}
        
        # Dictionary of parameters and their types required by the data sender
        self.sender_params = {
              'host'                         : ['/config/DataBuffer/DaqHost', str],
              'port'                         : ['/config/DataBuffer/DaqPort', int],
              'microslice_size_rollover'     : ['/config/Microslice/Duration', int],
              'millislices'                  : ['/config/Emulator/NumMillislices', int],
              'microslices'                  : ['/config/Emulator/NumMicroslices', int],
              'rate'                         : ['/config/Emulator/SendRate', float],
              'payload_mode'                 : ['/config/Emulator/PayloadMode', int],
              'trigger_mode'                 : ['/config/Emulator/TriggerMode', int],
              'fragment_microslice_at_ticks' : ['/config/Emulator/FragmentUSlice', int],
              'repeat_microslices'           : ['/config/Emulator/SendRepeats', int],
              'debug_partial_recv'           : ['/config/Emulator/SendByByte', int]
              }
        
    def parse_request(self, cmd_data):
       
        cmd_ok = True
        reply = ""
        
        try: 
            root = ElementTree.fromstring(cmd_data)
        except ElementTree.ParseError as e:
            cmd_ok = False
            reply = e
        else:    
            if root.tag == "system":
                for child in root:
                    if child.tag == 'command':
                        (cmd_ok, reply) = self.do_command(child)
                    elif child.tag == "config":
                        (cmd_ok, reply) = self.do_config(child)
                    else:
                        cmd_ok = False
                        reply = "Unrecognised child tag in XML: {}".format(child.tag)
                        break
            else:
                cmd_ok = False
                reply = "Illegal root tag in XML: {}".format(root.tag)
                
        
        # TODO this response needs to be in XML format like PENN will produce
        if cmd_ok:
            response = "<system><ok>{}</ok></system>\n".format(reply)
        else:
            response = "<system><error>{}</error></system>\n".format(reply)   
        print "  >>>", response
        return response
    
    def do_command(self, cmd_element):
        
        # Iterate over child elements of req_command tag
        for child in cmd_element:
            req_command = child.tag
            print "Got req_command", req_command
        
            if req_command in self.commands:
                (cmd_ok, reply) = self.commands[req_command](child)
            else:
                cmd_ok = False
                reply  = "Unrecognised command: {}".format(req_command)
                
        return (cmd_ok, reply)
    
    def do_config(self, config_element):
        
        self.processConfigElement_(config_element, "")
        
        print self.params
        return (True, "Config OK")
    
    def processConfigElement_(self, config_element, config_path):
        
        config_path = config_path + "/" + config_element.tag
        if 'index' in config_element.attrib:
            config_path = config_path + '#' + config_element.attrib['index']
        
        if not list(config_element):
            #print "Config element: ", config_path, "value", config_element.text
            self.params[config_path] = config_element.text
        else:
            for config_child in config_element:
                self.processConfigElement_(config_child, config_path)
                        
    def hardReset_cmd(self, args):
        
        # Reset configuration parameters
        self.params = {}
        
        return (True, "HardReset command OK")
    
    def softReset_cmd(self, args):
        
        # On real PENN this resets counters etc
        return (True, "SoftReset command OK")
    
    def readXmlFile_cmd(self, args):
        
        xmlFile = ""
                
        try:
            if args.text == None:
                raise Exception("No XML config file path provided")
         
            xmlFile = args.text
            
            try:
                xmlConfigTree = ElementTree.parse(xmlFile)
                xmlConfigRoot = xmlConfigTree.getroot()
            except IOError as e:
                raise Exception("XML config file loading failed: {}".format(e))
            except ElementTree.ParseError as e:
                raise Exception("XML config file parsing failed: {}".format(e))
            
            if xmlConfigRoot.tag != "system":
                raise Exception("Expected XML config file to have <system> root element, but found {}".format(xmlConfigRoot.tag))
            
            for config_element in xmlConfigRoot.iter('config'):
                self.processConfigElement_(config_element, "")
            
        except Exception as e:
            cmd_ok = False
            reply = e
        else:
            cmd_ok = True
            reply = "XML config file {} loaded OK".format(xmlFile)
                
        return (cmd_ok, reply)
    
    def setRunState_cmd(self, args):
        
        runState = ""
        try:
            if args.text == None:
                raise Exception("SetRunState: no run state specified")
            
            runState = args.text
            
            if runState == 'Enable':
                self.doRunStart()
            elif runState == "Stopped":
                self.doRunStop()
            else:
                raise Exception("SetRunState: illegal run state {} specified".format(runState))
                
        except Exception as e:
            cmd_ok = False
            reply = e
        else:
            cmd_ok = True
            reply = "Run state set to {}".format(runState)
            
        return (cmd_ok, reply)
    
    def doRunStart(self):
        
        print "Got run start command"
        
        for (param, (param_name, param_type)) in self.sender_params.iteritems():
            if param_name in self.params:
                param_val = param_type(self.params[param_name])
                try:
                    getattr(self.sender, 'set_' + param)(param_val)
                except AttributeError as e:
                    raise Exception("Attempted to set illegal parameter for data sender: {}".format(e))
        
        (cmd_ok, reply) = self.sender.run()
        if not cmd_ok:
            raise Exception("Data sender start failed: {}".format(reply))
    
    def doRunStop(self):
        
        print "Got run stop command"
        (cmd_ok, reply) = self.sender.stop()
        
        if not cmd_ok:
            raise Exception("Data sender stop failed: {}".format(reply))
