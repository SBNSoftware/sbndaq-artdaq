#!/usr/bin/env python3
"""Decode the per-board per-event record files written by CAENV1730Readout
(v1730_events_frag<id>_run<run>_<host>_pid<pid>.bin) into CSV tables.

File layout (little endian):
    64-byte header   char[8] magic "V1730EVT", u32 version, u32 fragmentId, u32 runNumber,
                     u32 recordLength (samples), u32 expectedEventSizeWords, u32 channelEnableMask,
                     u64 startTimeNs, u32 boardSerial, u32 reserved[5]
    40-byte records  u32 eventCounter, u32 triggerTimeTag (8 ns ticks), u32 eventSizeWords, u32 dTTT
                     (0xFFFFFFFF = unknown), u64 hostPollEndNs, u32 flags, u16 nReadsInPoll,
                     u16 eventsStoredAtPoll, u32 returnedBytes, u32 reserved
    flags            bit0 dropped by the size check, bit1 tail-repeat corruption seen, bit2 short
                     event (size != expected), bit3 overlapped (dTTT < one record); bits 16-31 =
                     per-channel mask of channels whose last 4 words repeat the 4 before them

Usage:
    decode_v1730_events.py <outdir> <files...>

Writes to <outdir>:
    events_<run>_frag<id>.csv     one row per record, with the offset from stack pulse 1 of its 250 ms period
    stacks_<run>_frag<id>.csv     one row per pulse, grouped by global trigger (cluster within +-1 ms of a
                                  stack), stack_id = event counter of stack pulse 1
and prints per file: number of events, overlapped fraction, gap histogram of the smallest gaps,
fraction of global triggers with a 48-58-tick pair, with a pair <= 20 ticks, tail-repeat events.
"""
import csv, os, struct, sys
import numpy as np

HDR = struct.Struct("<8sIIIIIIQI5I")
REC = struct.Struct("<IIIIQIHHII")
PERIOD = 31250000   # ticks per 250 ms (4 Hz global trigger)
SECOND = 125000000  # ticks per second (TTT resets on PPS)
STACK_GAPS = ((510, 516), (350, 356), (1122, 1128))


def read_file(path):
    data = open(path, "rb").read()
    h = HDR.unpack_from(data, 0)
    assert h[0] == b"V1730EVT", path
    hdr = dict(version=h[1], fragmentId=h[2], run=h[3], recordLength=h[4], expectedWords=h[5],
               channelMask=h[6], startTimeNs=h[7], serial=h[8])
    n = (len(data) - HDR.size) // REC.size
    recs = np.array([REC.unpack_from(data, HDR.size + i * REC.size) for i in range(n)], dtype=np.int64)
    return hdr, recs


def stack_phase(ttt, dttt, size, expected):
    """phase of stack pulse 1 inside the 250 ms period, from pulse-2 candidates (gap 512 and size 4+32*gap)."""
    sel = (dttt >= 510) & (dttt <= 516) & (size == 4 + 32 * dttt)
    if sel.sum() < 3:
        return None
    ph = (ttt[sel] - dttt[sel]) % PERIOD
    vals, cnts = np.unique(ph // 4, return_counts=True)
    return int(vals[np.argmax(cnts)] * 4)


def main():
    outdir, files = sys.argv[1], sys.argv[2:]
    os.makedirs(outdir, exist_ok=True)
    for path in files:
        hdr, r = read_file(path)
        if len(r) == 0:
            print(path, ": empty"); continue
        ec, ttt, size, dttt, host, flags, nread, stored, rbytes = (r[:, k] for k in range(9))
        ticks_per_record = hdr["recordLength"] // 4
        phase = stack_phase(ttt, dttt, size, hdr["expectedWords"])
        off = np.full(len(r), np.iinfo(np.int64).min)
        if phase is not None:
            o = (ttt - phase) % PERIOD
            off = np.where(o > PERIOD // 2, o - PERIOD, o)
        tag = f"{hdr['run']}_frag{hdr['fragmentId']}"

        # ---- events CSV
        with open(os.path.join(outdir, f"events_{tag}.csv"), "w", newline="") as fh:
            w = csv.writer(fh)
            w.writerow(["run", "fragmentId", "eventCounter", "TTT", "dTTT", "eventSizeWords", "hostPollEndNs", "flags",
                        "dropped", "tailRepeat", "short", "overlapped", "tailMask", "nReadsInPoll", "storedAtPoll",
                        "returnedBytes", "offset_from_stack1_ticks"])
            for i in range(len(r)):
                w.writerow([hdr["run"], hdr["fragmentId"], ec[i], ttt[i], dttt[i] if dttt[i] != 0xFFFFFFFF else "",
                            size[i], host[i], flags[i], flags[i] & 1, (flags[i] >> 1) & 1, (flags[i] >> 2) & 1,
                            (flags[i] >> 3) & 1, f"0x{(flags[i] >> 16) & 0xFFFF:04x}", nread[i], stored[i], rbytes[i],
                            off[i] if phase is not None else ""])

        # ---- stacks CSV: group consecutive events into clusters around each stack (|offset| <= 1 ms)
        nstack = 0; with50 = 0; with20 = 0
        if phase is not None:
            with open(os.path.join(outdir, f"stacks_{tag}.csv"), "w", newline="") as fh:
                w = csv.writer(fh)
                w.writerow(["run", "fragmentId", "stack_id", "seq", "eventCounter", "TTT", "offset_ticks", "dTTT",
                            "eventSizeWords", "class"])
                i = 0
                while i < len(r):
                    if abs(off[i]) > 125000:
                        i += 1; continue
                    j = i
                    while j + 1 < len(r) and abs(off[j + 1]) <= 125000 and (ec[j + 1] - ec[j]) <= 40 and ttt[j + 1] >= ttt[j]:
                        j += 1
                    # stack pulse 1 = pulse nearest offset 0
                    k0 = i + int(np.argmin(np.abs(off[i:j + 1])))
                    gaps = dttt[i:j + 1]
                    has50 = np.any((gaps >= 48) & (gaps <= 58)); has20 = np.any(gaps <= 20)
                    nstack += 1; with50 += has50; with20 += has20
                    for s, k in enumerate(range(i, j + 1)):
                        g = int(dttt[k])
                        cls = ("stack1" if k == k0 else "stack2" if 510 <= g <= 516 else "stack3" if 350 <= g <= 356
                               else "stack4" if 1122 <= g <= 1128 else "pair50" if 48 <= g <= 58
                               else "pair_le20" if g <= 20 else "full" if size[k] == hdr["expectedWords"] else "overlap")
                        w.writerow([hdr["run"], hdr["fragmentId"], ec[k0], s, ec[k], ttt[k], off[k], g, size[k], cls])
                    i = j + 1

        # ---- summary
        good = (flags & 1) == 0
        ov = good & (dttt != 0xFFFFFFFF) & (dttt < ticks_per_record)
        small = dttt[good & (dttt != 0xFFFFFFFF) & (dttt <= 64)]
        hist = np.bincount(small, minlength=65) if len(small) else np.zeros(65, int)
        print(f"{os.path.basename(path)}: run {hdr['run']} frag {hdr['fragmentId']} S/N {hdr['serial']}: {len(r)} events, "
              f"{ov.sum()} overlapped ({100*ov.mean():.1f}%), {int(((flags >> 1) & 1).sum())} tail-repeat, "
              f"{int((flags & 1).sum())} dropped")
        print(f"   gaps <= 64 ticks (tick: count): " + ", ".join(f"{t}:{c}" for t, c in enumerate(hist) if c))
        if phase is not None:
            print(f"   {nstack} global triggers; with a 48-58-tick pair: {with50} ({100*with50/max(nstack,1):.1f}%); "
                  f"with a pair <= 20 ticks: {with20} ({100*with20/max(nstack,1):.2f}%)")
        else:
            print("   no stack phase found (fewer than 3 pulse-2 candidates)")


if __name__ == "__main__":
    main()
