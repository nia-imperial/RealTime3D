# requirements:
# numpy, matplotlib, imageio, tqdm

from typing import List
from pathlib import Path
import numpy as np
import imageio


def dat2tiff(dat_file_path, display=False, byte_order='<', index_order='F', save_neg=False, dry_run=False):
    print(Path.cwd())
    dat_file_path = Path(dat_file_path)
    with open(dat_file_path, 'rb') as file:
        data = file.read()

    header = np.frombuffer(data[:16], dtype=np.int_)
    nrows = header[0]
    ncols = header[1]

    if not header[2] <= 1 or not header[3] <= 1:
        print(f'File {dat_file_path} byte format cannot be read.')
        return

    dt = np.dtype(np.float64)
    dt = dt.newbyteorder(byte_order)
    dat = np.frombuffer(data[16:], dtype=dt)

    dat = dat.reshape(ncols, nrows, order=index_order).T

    tiff_file_name = dat_file_path.stem + '.tiff'
    tiff_file_path = dat_file_path.with_name(tiff_file_name)
    if dry_run:
        print(dat_file_path.name, '->', tiff_file_path.name)
    else:
        imageio.imwrite(tiff_file_path.as_posix(), dat)

    if save_neg and not dry_run:
        neg_tiff_file_name = dat_file_path.stem + '_n.tiff'
        neg_tiff_file_path = dat_file_path.with_name(neg_tiff_file_name)
        imageio.imwrite(neg_tiff_file_path.as_posix(), dat * -1)


def dir_dat2tiff(directory_path, **kwargs):
    directory_path = Path(directory_path)
    for f in list(directory_path.glob('*.dat')):
        dat2tiff(f, **kwargs)


def renumber_files(directory_path, start, end, dry_run=False):
    directory_path = Path(directory_path)

    files: List[Path] = list(directory_path.iterdir())

    number_map = {i: 0 for i in range(start, end + 1)}
    number_map = {i: k for i, (k, v) in enumerate(number_map.items())}

    i = start
    for n, f in enumerate(files):
        file_parts = f.stem.split('_')
        if int(file_parts[3]) >= start:
            continue

        file_parts[3] = str(number_map[int(file_parts[3])])
        file_parts[4] = str(number_map[int(file_parts[4])])

        new_file_stem = '_'.join(file_parts)
        new_file_name = new_file_stem + f.suffix
        new_file_path = f.with_name(new_file_name)
        if dry_run:
            print(f'{f} -> {new_file_path.as_posix()}')
        else:
            f.rename(new_file_path)
        if n % 7 == 0:
            i += 1
        continue


if __name__ == '__main__':
    import argparse

    parser = argparse.ArgumentParser(description='Convert RT3D .dat files into .tiff files')
    parser.add_argument('--data-path', '-i', type=Path, help='Path to file or directory for conversion.')
    parser.add_argument('--show-display', action='store_true', dest='display',
                        help='Shows the images as they are converted.')
    parser.add_argument('--byte-order', '-b', choices=['<', '>', '='], default='<')
    parser.add_argument('--index-order', '-o', choices=['C', 'F'], default='F')
    parser.add_argument('--save-negatives', action='store_true', dest='save_neg',
                        help='Saves the negative in addition to regular conversion.')
    parser.add_argument('--dry-run', '-D', action='store_true')

    subparsers = parser.add_subparsers(description='valid subcommands',
                                       dest='renumber_parser')
    # parser for renumber_files
    parser_renumbering = subparsers.add_parser('Renumber', help='Renumber tiff files in directory for a given range.')
    parser_renumbering.add_argument('--file-path', '-i', type=Path, help='Path to directory for renumbering.',
                                    dest='directory_path', required=True)
    parser_renumbering.add_argument('--start', '-s', type=int, required=True)
    parser_renumbering.add_argument('--end', '-e', type=int, required=True)
    parser_renumbering.add_argument('--dry-run', '-D', action='store_true')

    args = parser.parse_args()
    if args.renumber_parser is not None:
        known_args, _ = parser_renumbering.parse_known_args()
        renumber_files(**known_args.__dict__)
    else:
        kargs = args.__dict__
        kargs.pop('renumber_parser')
        if args.data_path.is_dir():
            dir_dat2tiff(kargs.pop('data_path'), **kargs)
        else:
            dat2tiff(kargs.pop('data_path'), **kargs)

