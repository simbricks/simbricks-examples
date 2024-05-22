import sys
from PIL import Image
import os


if len(sys.argv) != 3:
    print(
        "This module allows you to reencode images in the format that the JPEG"
        " decoder expects, namely 4:4:4 subsampling with unoptimized Huffman"
        " tables."
    )
    print("Usage: encode_jpeg_444_unopt src dst")
    sys.exit(1)


source_folder = sys.argv[1]
destination_folder = sys.argv[2]

for file_name in os.listdir(source_folder):
    if file_name.endswith(".jpg"):
        with Image.open(os.path.join(source_folder, file_name)) as img:
            # Getting image dimensions
            width, height = img.size
            # Saving the image with specified parameters
            destination_path = os.path.join(destination_folder, file_name)
            img.save(
                destination_path,
                "JPEG",
                quality=90,
                optimize=False,
                progressive=False,
                subsampling="4:4:4",
            )

            compressed_size = os.path.getsize(destination_path)

            print(
                f"Image: {file_name}, Width: {width}, Height: {height}, Source"
                f" Size:  Compressed Size: {compressed_size} bytes"
            )
