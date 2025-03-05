#!/usr/bin/env python3
import heapq
import math
import random
import sys

if len(sys.argv) != 7:
    print("Usage: {} <total_duration> <balanced_duration> <imbalanced_duration> <num_actors> <imbalance_percentage> <event_rate>".format(sys.argv[0]))
    sys.exit(1)

total_duration       = float(sys.argv[1])
balanced_duration    = float(sys.argv[2])
imbalanced_duration  = float(sys.argv[3])
num_actors           = int(sys.argv[4])
imbalance_percentage = float(sys.argv[5])
event_rate           = float(sys.argv[6])

cycle_duration = balanced_duration + imbalanced_duration
hotspot_partition = max(1, int(math.floor(num_actors * imbalance_percentage / 100.0)))
ofile = f"trace_{total_duration:.1f}s_{balanced_duration:.1f}b_{imbalanced_duration:.1f}i_{num_actors}actors_{imbalance_percentage:.1f}imb_{event_rate:.1f}rate.trace"

print(f"Generating random trace for {num_actors} actors with an imbalance percentage of {imbalance_percentage}% (hotspot group: first {hotspot_partition} actors).")
print(f"Total duration: {total_duration:.2f}s. Each actor produces events as a Poisson process with rate {event_rate:.2f} events/sec.")
print(f"Balanced phase duration: {balanced_duration:.2f}s, Imbalanced phase duration: {imbalanced_duration:.2f}s, Cycle period: {cycle_duration:.2f}s.")
print(f"Writing trace to {ofile}")

# Initialize a heap to merge event streams from all actors.
heap = []
for actor in range(num_actors):
    first_event_time = random.expovariate(event_rate)
    if first_event_time <= total_duration:
        heapq.heappush(heap, (first_event_time, actor))

with open(ofile, "w") as f:
    f.write("src_ts, src_id, dst_ts, dst_id\n")
    evt_count = 0
    while heap:
        timestamp, sender = heapq.heappop(heap)
        if evt_count % 5000 == 0:
            print(f'\rTime: {timestamp}/{total_duration} ', end='', flush=True)
        if timestamp > total_duration:
            continue

        phase_time = timestamp % cycle_duration
        if phase_time < balanced_duration:
            # In balanced phase, compute a valid receiver without building a full list.
            r = random.randint(0, num_actors - 2)
            receiver = r if r < sender else r + 1
        else:
            # In imbalanced phase, the receiver is selected from the hotspot subset.
            # If the sender is within the hotspot group and the group has more than one member, avoid self-messaging.
            if sender < hotspot_partition and hotspot_partition > 1:
                r = random.randint(0, hotspot_partition - 2)
                receiver = r if r < sender else r + 1
            else:
                receiver = random.randint(0, hotspot_partition - 1)

        f.write(f"{timestamp:.4f},{sender},{timestamp+0.001:.4f},{receiver}\n")
        evt_count += 1

        # Generate and push the next event for the current sender.
        next_time = timestamp + random.expovariate(event_rate)
        if next_time <= total_duration:
            heapq.heappush(heap, (next_time, sender))

print("Done.")
