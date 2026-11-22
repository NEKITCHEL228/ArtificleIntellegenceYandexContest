import argparse
from collections import defaultdict
import json
import math
from pathlib import Path

import numpy as np


RANGE_NAMES = ('0-10m', '10-25m', '25-60m')


def evaluate(data_dir, predictions):
    """Return R, P, F, S and diagnostic counts."""
    files = sorted(Path(data_dir).glob('*.npz'), key=lambda p: p.name)
    if not files:
        raise ValueError('No NPZ files found in the data directory')
    counts = []
    for path in files:
        with np.load(path, allow_pickle=False) as raw:
            required = {'points', 'frame_offsets', 'labels', 'object_ids'}
            if not required.issubset(raw.files):
                raise ValueError(f'{path.name}: labeled train/val files are required')
            offsets = raw['frame_offsets']
            if (offsets.ndim != 1 or len(offsets) < 2
                    or not np.issubdtype(offsets.dtype, np.integer)
                    or offsets[0] != 0 or np.any(np.diff(offsets) < 0)
                    or (len(offsets) - 1) % 10):
                raise ValueError(f'{path.name}: invalid offsets or incomplete ten-frame clips')
            counts.append(int(offsets[-1]))

    total = sum(counts)
    expected_bytes = (total + 7) // 8
    with open(predictions, 'rb') as stream:
        packed = stream.read(expected_bytes + 1)
    if len(packed) != expected_bytes:
        raise ValueError(f'Incorrect prediction size: expected {expected_bytes} bytes, got {len(packed)}')
    if total % 8 and packed[-1] >> (total % 8):
        raise ValueError('Nonzero padding bits')
    packed = np.frombuffer(packed, dtype=np.uint8)

    objects = defaultdict(lambda: [0, 0])
    opportunities, false_alarms = set(), set()
    precisions = []
    cursor = frames = tp = fp = fn = 0
    for path, count in zip(files, counts):
        with np.load(path, allow_pickle=False) as raw:
            xyz, offsets, labels, ids = (raw[k] for k in
                                        ('points', 'frame_offsets', 'labels', 'object_ids'))
        if (xyz.shape != (count, 3) or labels.shape != (count,) or ids.shape != (count,)
                or not np.isin(labels, [0, 1]).all()
                or np.any(ids[labels == 1] < 0) or np.any(ids[labels == 0] != -1)):
            raise ValueError(f'{path.name}: invalid points, labels, or object IDs')
        end = cursor + count
        prediction = np.unpackbits(packed[cursor // 8:(end + 7) // 8], bitorder='little')
        prediction = prediction[cursor % 8:cursor % 8 + count].astype(bool)
        cursor = end
        for frame, (lo, hi) in enumerate(zip(offsets[:-1], offsets[1:])):
            positive = labels[lo:hi].astype(bool)
            selected = prediction[lo:hi]
            frame_ids = ids[lo:hi]
            selected_count = int(np.count_nonzero(selected))
            hits = int(np.count_nonzero(positive & selected))
            clip = (path.stem, frame // 10)
            if not positive.any():
                opportunities.add(clip)
                if selected_count:
                    false_alarms.add(clip)
            if selected_count:
                precisions.append(hits / selected_count)
            distance = np.linalg.norm(xyz[lo:hi], axis=1)
            for obj in np.unique(frame_ids[positive]):
                eligible = positive & (frame_ids == obj)
                group = int(np.searchsorted([10., 25.], np.median(distance[eligible]), side='right'))
                # Each second is a separate evaluation unit, including in train/val.
                stats = objects[clip + (int(obj), group)]
                stats[0] += 1
                stats[1] += int(np.any(selected & eligible))
            tp += hits
            fp += selected_count - hits
            fn += int(np.count_nonzero(positive)) - hits
            frames += 1

    by_range, units = {}, {}
    for group, name in enumerate(RANGE_NAMES):
        values = [hit / n for key, (n, hit) in objects.items() if key[-1] == group]
        by_range[name] = math.fsum(values) / len(values) if values else None
        units[name] = len(values)
    present = [r for r in by_range.values() if r is not None]
    recall = math.fsum(present) / len(present) if present else 0.
    precision = math.fsum(precisions) / len(precisions) if precisions else 0.
    false_alarm_rate = len(false_alarms) / len(opportunities) if opportunities else 0.
    score = recall * precision * (1 - false_alarm_rate) ** 2
    return dict(R=recall, P=precision, F=false_alarm_rate, S=score,
                recall_by_range=by_range,
                object_clip_range_counts=units, sequences=len(files), frames=frames,
                selected_frames=len(precisions), false_alarm_clips=len(false_alarms),
                false_alarm_opportunity_clips=len(opportunities), tp=tp, fp=fp, fn=fn)


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--data', required=True, type=Path, help='Directory with labeled train or val NPZ files')
    parser.add_argument('--predictions', required=True, type=Path, help='Packed binary predictions for that directory')
    args = parser.parse_args()
    try:
        result = evaluate(args.data, args.predictions)
    except (OSError, ValueError, KeyError) as error:
        parser.exit(2, f'Error: {error}\n')
    print(json.dumps({key: result[key] for key in ('R', 'P', 'F', 'S')},
                     indent=2, allow_nan=False))


if __name__ == '__main__':
    main()
