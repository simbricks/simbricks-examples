# Copyright 2024 Max Planck Institute for Software Systems, and
# National University of Singapore
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
"""This script extracts and opens the base64 encoded images from the
jpeg_decoder experiment."""

import sys
import json
import typing as tp
import re
import base64
import numpy as np
import matplotlib.pyplot as plt


def show_base64_encoded_imgs(stdout: tp.List[str]) -> None:
    # parse lines from output json and edit them
    line_idx = 0
    figure = 1
    while line_idx < len(stdout):
        start_idx = None
        for i in range(line_idx, len(stdout)):
            if stdout[i].startswith("image dump begin"):
                split = stdout[i].split(" ")
                width = int(split[3])
                height = int(split[4])
                # we also want to skip the next two lines
                start_idx = i + 3
                break

        if start_idx is None:
            break

        end_idx = None
        for i in range(start_idx, len(stdout)):
            if stdout[i].startswith("image dump end"):
                # eliminate the line where the echo command itself is printed
                end_idx = i - 1
                break

        if end_idx is None:
            print('Couldn\'t find end string ("image dump end")')
            break

        eliminate = [
            r"\[ *\d+(\.\d+)?\] random: crng init done"
        ]

        def edit_line(line: str) -> str:
            line = line.removesuffix("\r")
            for pattern in eliminate:
                line = re.sub(pattern, "", line)
            return line

        edited_lines = list(map(edit_line, stdout[start_idx:end_idx]))

        # convert base64 encoded image to 3-Byte RGB and show it
        decoded_img = base64.b64decode("".join(edited_lines), validate=True)
        img_rgb565 = np.frombuffer(decoded_img, dtype=np.uint16)
        assert len(img_rgb565) == width * height
        img_rgb888 = np.empty((height, width, 3), dtype=np.uint8)

        MASK5 = 0b011111
        MASK6 = 0b111111
        for y in range(height):
            for x in range(width):
                idx_565 = y * width + x
                img_rgb888[y, x, 2] = (img_rgb565[idx_565] & MASK5) << 3
                img_rgb888[y, x, 1] = ((img_rgb565[idx_565] >> 5) & MASK6) << 2
                img_rgb888[y, x, 0] = (
                    (img_rgb565[idx_565] >> 5 + 6) & MASK5
                ) << 3

        plt.figure(figure)
        figure += 1
        plt.imshow(img_rgb888)
        line_idx = end_idx + 1

    plt.show()


def extract_decoding_durations(stdout: tp.List[str]) -> None:
    durations = dict()
    i = 0
    while i < len(stdout):
        # find image name
        while i < len(stdout):
            if stdout[i].startswith("starting decode of image"):
                split = stdout[i].split(" ")
                img_name = split[4].removesuffix("\r")
                i += 1  # continue in next line
                break
            else:
                i += 1

        # find line with duration
        while i < len(stdout):
            if stdout[i].startswith("duration:"):
                split = split = stdout[i].split(" ")
                durations[img_name] = int(split[1])
                i += 1  # continue in next line
                break
            else:
                i += 1

    # print durations
    if durations:
        print("Durations in ns for decoding images:")
        for img, dur in sorted(durations.items()):
            print(img, dur)


def main():
    if len(sys.argv) != 2:
        print("Usage: process_exp_output.py exp_out.json")
        sys.exit(1)

    with open(sys.argv[1], mode="r", encoding="utf-8") as file:
        exp_out = json.load(file)
        stdout: tp.List[str] = exp_out["sims"]["host."]["stdout"]
        extract_decoding_durations(stdout)
        print("Rendering the raw base64-encoded images with matplotlib.pyplot.show() ...")
        show_base64_encoded_imgs(stdout)


if __name__ == "__main__":
    main()
