import datetime

class NovaTimestamp(object):

    # NOvA epoch is 00:00:00 UTC 1st Jan, 2010
    epoch = datetime.datetime(2010, 1, 1, 0, 0, 0, 0)

    # UTC epoch is 00:00:00 UTC 1st Jan, 1970
    utc_epoch = datetime.datetime(1970, 1, 1, 0, 0, 0, 0)

    # NOvA clock is 64 MHZ, = 15.6ns tick
    ticks_per_sec = 64E6

    #NOvA clock is 56 bits

    def __init__(self, posix_time=None, override=None):
        if posix_time == None:
            self.now = datetime.datetime.now()
        else:
            self.now = datetime.datetime.fromtimestamp(posix_time)

        self.since_epoch       = (self.now - NovaTimestamp.epoch).total_seconds()
        self.ticks_since_epoch = int(self.since_epoch * NovaTimestamp.ticks_per_sec)

        if(override):
            self.ticks_since_epoch = override
        self.epoch_to_timestamp()

    def epoch_to_timestamp(self):
        self.timestamp_low     = self.ticks_since_epoch & 0xFFFFFFFF #32 bits
        self.timestamp_high    = (self.ticks_since_epoch >> 32) & 0xFFFFFF #24 bits
        self.timestamp_short   = self.ticks_since_epoch & 0xFFFFFFF #28 bits

    def increment(self):
        self.ticks_since_epoch += 1
        self.epoch_to_timestamp()

    def nova_time_from_human_time(self, year, month, day, hour, minute, second, microsecond=0):
        now               = datetime.datetime(year, month, day, hour, minute, second, microsecond)
        since_epoch       = (now - NovaTimestamp.epoch).total_seconds()
        ticks_since_epoch = int(self.since_epoch * NovaTimestamp.ticks_per_sec)
        print 'Nova timestamp is between {:d} and {:d}'.format(ticks_since_epoch, int(ticks_since_epoch + NovaTimestamp.ticks_per_sec - 1))
            #ticks_since_epoch, 'and', (ticks_since_epoch + NovaTimestamp.ticks_per_sec - 1)
        return ticks_since_epoch

    def nova_time_from_datetime(self, dt):
        return self.nova_time_from_human_time(dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second, dt.microsecond)
        
    def human_time_from_nova_time(self, novatime):
        now_nova = datetime.datetime.fromtimestamp(novatime / NovaTimestamp.ticks_per_sec)
        delta = NovaTimestamp.epoch - NovaTimestamp.utc_epoch
        now_utc  = now_nova + datetime.timedelta(days=delta.days)
        print 'The real time is', now_utc
        return now_utc

if __name__ == '__main__':
    nt = NovaTimestamp()
    ticks_since_epoch = nt.nova_time_from_datetime(datetime.datetime.now())
    nt.human_time_from_nova_time(ticks_since_epoch)
