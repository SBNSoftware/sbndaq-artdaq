#!/usr/bin/env python3
"""Decode a CAEN V1730 ReadData-failure incident dump written by CAENV1730Readout
(test instrumentation for the error-22 / CAEN_DGTZ_OutOfMemory investigation).

Usage:  decode_v1730_dump.py v1730_frag8201_run14760_sbnd-pmt01_pid12345_20260920-031522.417.bin [--samples]

Binary layout: a sequence of records, each with an 88-byte header
    char[8]  magic          "V1730DMP"
    uint32   type           1 = ring event, 2 = failing block, 3 = FIFO pop words
    uint32   version        1
    uint64   payloadBytes
    uint64   meta[8]
followed by payloadBytes of payload.

meta per type
    1: pollBegin_ns, pollEnd_ns, eventCounter, eventSizeWords, TTT, dTTT,
       (storedAtPoll<<32)|readIndexInPoll, flags (bit0 = dropped by size check)
    2: blockIndex, blockSize, sentinelExtentBytes, sentinelChangedBytes,
       returnCode (as int64), (EVENT_SIZE<<32)|EVENT_STORED, ACQ_STATUS, nReadsThisPoll
    3: nWords, stopCode (as int64), 0...
"""
import struct, sys

HDR = struct.Struct('<8sIIQ8Q')
SENTINEL = 0xDEADBEEF


def words_of(payload):
    n = len(payload) // 4
    return struct.unpack('<%dI' % n, payload[:n * 4])


def describe_header(w):
    """Interpret the first 4 words as a V1730 event header."""
    if len(w) < 4:
        return 'too short for a header'
    marker = w[0] >> 28
    size = w[0] & 0x0FFFFFFF
    return ('marker=0x%X size=%d words, boardId=%d BF=%d pattern=0x%04X chMask=0x%02X, '
            'eventCounter=%d, TTT=%d' % (marker, size, w[1] >> 27, (w[1] >> 26) & 1,
                                         (w[1] >> 8) & 0xFFFF, w[1] & 0xFF, w[2] & 0xFFFFFF, w[3]))


def sample_stats(w):
    """Treat words as pairs of 14-bit samples; return fraction that look like samples and mean."""
    if not w:
        return 0.0, 0.0
    ok = 0
    tot = 0
    n = 0
    for x in w:
        lo, hi = x & 0xFFFF, x >> 16
        if (lo & 0xC000) == 0 and (hi & 0xC000) == 0:
            ok += 1
            tot += lo + hi
            n += 2
    return ok / len(w), (tot / n if n else 0.0)


def to_signed(u):
    return u - (1 << 64) if u >= (1 << 63) else u


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(1)
    show_samples = '--samples' in sys.argv
    data = open(sys.argv[1], 'rb').read()
    pos = 0
    rec_no = 0
    prev_ttt = None
    while pos + HDR.size <= len(data):
        magic, rtype, version, nbytes, *meta = HDR.unpack_from(data, pos)
        if magic != b'V1730DMP':
            print('bad magic at offset %d, stopping' % pos)
            break
        pos += HDR.size
        payload = data[pos:pos + nbytes]
        pos += nbytes
        w = words_of(payload)
        if rtype == 1:
            pb, pe, evc, esz, ttt, dttt, packed, flags = meta
            stored, ridx = packed >> 32, packed & 0xFFFFFFFF
            frac, mean = sample_stats(w[4:])
            print('[%2d] RING EVENT  bytes=%-7d evc=%-9d size=%d words TTT=%d dTTT=%s ticks '
                  'readInPoll=%d storedAtPoll=%s flags=%d poll=%d..%d ns (%.1f ms)' % (
                      rec_no, nbytes, evc, esz, ttt,
                      ('%d' % dttt if dttt != 0xFFFFFFFF else 'n/a'), ridx,
                      ('%d' % stored if stored != 0xFFFFFFFF else 'n/a'), flags, pb, pe, (pe - pb) / 1e6))
            print('      header: %s' % describe_header(w))
            print('      payload after header: %.1f%% sample-like words, mean sample %.0f ADC' % (100 * frac, mean))
            if esz * 4 != nbytes:
                print('      ** header size %d bytes != returned %d bytes' % (esz * 4, nbytes))
            prev_ttt = ttt
        elif rtype == 2:
            bidx, bsize, extent, changed, rc, packed, acq, nreads = meta
            esize, stored = packed >> 32, packed & 0xFFFFFFFF
            n_sentinel = sum(1 for x in w if x == SENTINEL)
            print('[%2d] FAILING BLOCK index=%d size=%d rc=%d nReadsThisPoll=%d EVENT_STORED=%d '
                  'EVENT_SIZE=0x%08X (%d words; hi16=%d lo16=0x%04X) ACQ_STATUS=0x%X' % (
                      rec_no, bidx, bsize, to_signed(rc), nreads, stored, esize, esize, esize >> 16,
                      esize & 0xFFFF, acq))
            print('      ReadData wrote %d bytes (extent %d); %d of %d words still sentinel' % (
                changed, extent, n_sentinel, len(w)))
            if extent:
                ww = w[:extent // 4]
                print('      first words: %s' % ' '.join('0x%08X' % x for x in ww[:16]))
                print('      as header: %s' % describe_header(ww))
                frac, mean = sample_stats(ww)
                print('      %.1f%% sample-like words, mean sample %.0f ADC' % (100 * frac, mean))
        elif rtype == 3:
            nw, rc = meta[0], to_signed(meta[1])
            frac, mean = sample_stats(w)
            hdr_like = sum(1 for x in w if (x >> 28) == 0xA)
            print('[%2d] FIFO POPS  %d words, stop code %d; %.1f%% sample-like (mean %.0f ADC), '
                  '%d words with 0xA nibble' % (rec_no, nw, rc, 100 * frac, mean, hdr_like))
            print('      first words: %s' % ' '.join('0x%08X' % x for x in w[:16]))
        else:
            print('[%2d] unknown record type %d (%d bytes)' % (rec_no, rtype, nbytes))
        if show_samples and rtype == 1 and len(w) > 4:
            # per-channel mean of the first 64 samples, assuming all enabled channels equal length
            body = w[4:]
            nch = 16
            per_ch = len(body) // nch
            for ch in range(nch):
                seg = body[ch * per_ch: ch * per_ch + 32]
                s = [v for x in seg for v in (x & 0x3FFF, (x >> 16) & 0x3FFF)]
                print('      ch%02d first-64-sample mean %.1f min %d max %d' % (ch, sum(s) / len(s), min(s), max(s)))
        rec_no += 1


if __name__ == '__main__':
    main()
