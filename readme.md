[//]: # (    Note that this file is written in Markdown, and is best viewed with a Markdown viewer/editor, such as https://dillinger.io, or directly on GitHub.     )








# Seals Volume Extractor

This CLI tool is a collection of utilities to work with raw .vol files, meant primarily for the analysis of seal skull scans.

## Features

- Extract image slices from volume
- Downscale volume samples
- TODO: Convert volume voxels to cubified polygon mesh
- TODO: Denoise volume
- TODO: Convert volume voxels to polygon mesh using marching cubes

## Build

On Windows, the project can be opened and compiled with Visual Studio directly. On other platforms, use
```sh
$ make
```

In order to be able to open large volume files, it's recommended to use a 64-bit arch.

## Usage

To extract image slices from a .vol file (raw binary file containing `w*h*d` floats, where `w` is the width, `h` is the height, `d` is the depth), use
```sh
$ ./seals-vol <filename.vol> <width> <height> <depth> <black/white threshold>
```
