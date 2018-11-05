import time
import struct
import socket
import errno
import threading

from pennDataFormats import *

class PennDataSender(object):

    def __init__(self, use_tcp=False):

        self.dest_host = '127.0.0.1' #localhost
        self.dest_port = 8989
        self.send_rate = 10.0
        self.num_millislices = 10
        self.num_microslices = 10
        self.payload_mode = 0
        self.trigger_mode = 0
        self.microslice_size_rollover = 10
        self.fragment_microslice_at_ticks = -1
        self.debug_partial_recv = False
        self.repeat_microslices = False

        self.use_tcp = use_tcp
        self.do_send = False

    def set_host(self, host):
        self.dest_host = host

    def set_port(self, port):
        self.dest_port = int(port)

    def set_repeat_microslices(self, repeat):
        self.repeat_microslices = bool(repeat)

    def set_rate(self, rate):
        self.send_rate = float(rate)

    def set_millislices(self, millislices):
        self.num_millislices = int(millislices)

    def set_microslices(self, microslices):
        self.num_microslices = int(microslices)

    def set_payload_mode(self, payload_mode):
        self.payload_mode = int(payload_mode)
        
    def set_trigger_mode(self, trigger_mode):
        self.trigger_mode = int(trigger_mode)
        
    def set_microslice_size_rollover(self, microslice_size_rollover):
        self.microslice_size_rollover = int(microslice_size_rollover)

    def set_fragment_microslice_at_ticks(self, fragment_microslice_at_ticks):
        self.fragment_microslice_at_ticks = int(fragment_microslice_at_ticks)

    def set_debug_partial_recv(self, debug_partial_recv):
        self.debug_partial_recv = bool(debug_partial_recv)

    def run(self):

        cmd_ok = True
        reply = ""

        try:
            if self.use_tcp:
                print "Opening TCP socket connection to host %s port %d" % (
                    self.dest_host, self.dest_port)
                self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                self.sock.connect((self.dest_host, self.dest_port))
            else:
                self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        except socket.error as sock_err:
            if sock_err.errno == errno.ECONNREFUSED:
                cmd_ok = False
                reply = "Connection failed: %s" % sock_err
                print reply
            else:
                cmd_ok = False
                reply = "Unexpected socket error: %s" % sock_err
                print reply
        else:
            self.do_send = True
            self.run_thread = threading.Thread(target=self.send_loop)
            self.run_thread.daemon = True
            self.run_thread.start()

        return cmd_ok, reply

    def send_loop(self):

        print "Sender loop starting: %d millislices of %d microslices at rate %.1f host %s port %d" % (
          self.num_millislices, self.num_microslices, self.send_rate, self.dest_host, self.dest_port)

        send_interval = 1.0 / self.send_rate

        uslice = PennMicroslice(sequence = 0, payload_mode = self.payload_mode, trigger_mode = self.trigger_mode, microslice_size_rollover = self.microslice_size_rollover, fragment_microslice_at_ticks = self.fragment_microslice_at_ticks)
        
        start_time = time.time()

        num_uslices_total = self.num_millislices * self.num_microslices
        num_uslices_sent = 0

        i = 0
        
        if self.repeat_microslices:
            message = uslice.pack()

        while self.do_send:
                
            next_time = time.time() + send_interval

            if not self.repeat_microslices:
                uslice.set_sequence_id(num_uslices_sent)
                message = uslice.pack()

                if not (i % 1000):
                    uslice.print_microslice()
                else:
                    uslice.print_microslice(only_header=True)
                i += 1

            try:
                if self.debug_partial_recv:
                    #send a byte at a time, with a nice long wait between them
                    for i in xrange(len(message)):
                        if self.use_tcp:
                            self.sock.sendall(message[i])
                        else:
                            self.sock.sendto(message[i], (self.dest_host, self.dest_port))
                        time.sleep(0.1)
                else:
                    if self.use_tcp:
                        self.sock.sendall(message)
                    else:
                        self.sock.sendto(message, (self.dest_host, self.dest_port))
            except socket.error as e:
                print "Could not send microslice. Exception {}. Stopping send".format(e)
                self.do_send = False
                break

            num_uslices_sent += 1
            
            if (not self.do_send) or ((num_uslices_total > 0) and (num_uslices_sent >= num_uslices_total)):
                break;
            
            wait_interval = next_time - time.time()
            if wait_interval > 0.0:
                time.sleep(wait_interval)

        elapsed_time = time.time() - start_time

        sent_rate = float(num_uslices_sent) / elapsed_time
        print "Sender loop terminating: sent %d microslices total in %f secs, rate %.1f Hz" % (
          num_uslices_sent, elapsed_time, sent_rate
        )

        if self.use_tcp:
            self.sock.close()
            
    def stop(self):
        
        cmd_ok = True
        reply = ""
        
        self.do_send = False
        
        self.run_thread.join()
        
        return cmd_ok, reply
