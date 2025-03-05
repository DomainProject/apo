#!/usr/bin/env python3

import random
import sys

if len(sys.argv) != 5:
    print("Usage: cmd <num_frames> <frame_rate> <num_tiles> <num_classifiers>")
    sys.exit(1)

num_frames      = int(sys.argv[1])
frame_rate      = float(sys.argv[2])
num_tiles       = int(sys.argv[3])
num_classifiers = int(sys.argv[4])

# These represent base durations for each type of task.
# We will add random variability around these base durations.
TILE_BASE_DURATION         = 1.01
CLASSIFICATION_BASE_DURATION= 3.03
BOUNDING_BASE_DURATION     = 5.05

# We can also define a small variation range (fraction of base duration)
# to apply as uniform noise around the base.
VARIATION_FACTOR = 0.5  # 50% random variability

frame_period = 1.0 / frame_rate
ofile = f"video_surveillance_{num_frames}f_{frame_rate}fps_{num_tiles}tiles_{num_classifiers}cls.trace"

print("Generating synthetic trace for a video surveillance scenario.")
print(f"|- Frames: {num_frames}")
print(f"|- Frame Rate: {frame_rate} fps")
print(f"|- Tiles per Frame: {num_tiles}")
print(f"|- Classifiers: {num_classifiers}")
print(f"Writing trace to {ofile}")

with open(ofile, "w") as f:
    f.write("src_ts,src_id,dst_ts,dst_id\n")

    # We maintain a global task_id that increments without gaps.
    task_id = 0

    for frame_id in range(num_frames):
        # The arrival_time indicates when the frame becomes available.
        arrival_time = frame_id * frame_period

        # Generate tile tasks with random durations
        for _ in range(num_tiles):
            tile_start = arrival_time
            # random duration around base
            variation  = TILE_BASE_DURATION * VARIATION_FACTOR
            tile_dur   = TILE_BASE_DURATION + random.uniform(-variation, variation)
            if tile_dur < 0:
                tile_dur = 0.001  # to avoid negative or zero durations
            tile_end = tile_start + tile_dur

            f.write(f"{tile_start:.4f},{frame_id},{tile_end:.4f},{task_id}\n")
            task_id += 1

        # Generate classification tasks with random durations
        for _ in range(num_classifiers):
            class_start = arrival_time
            variation   = CLASSIFICATION_BASE_DURATION * VARIATION_FACTOR
            class_dur   = CLASSIFICATION_BASE_DURATION + random.uniform(-variation, variation)
            if class_dur < 0:
                class_dur = 0.001
            class_end = class_start + class_dur

            f.write(f"{class_start:.4f},{frame_id},{class_end:.4f},{task_id}\n")
            task_id += 1

        # Generate bounding box generation task with random duration
        # For simplicity, we keep bounding_start at arrival_time + base offset
        # but add random variability in the final duration.
        bounding_start = arrival_time + 0.02
        variation      = BOUNDING_BASE_DURATION * VARIATION_FACTOR
        bounding_dur   = BOUNDING_BASE_DURATION + random.uniform(-variation, variation)
        if bounding_dur < 0:
            bounding_dur = 0.001
        bounding_end   = bounding_start + bounding_dur

        f.write(f"{bounding_start:.4f},{frame_id},{bounding_end:.4f},{task_id}\n")
        task_id += 1

print("Done.")
