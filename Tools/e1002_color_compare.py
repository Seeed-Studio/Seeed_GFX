#!/usr/bin/env python3
"""
Generate reTerminal E1002 6-color conversion previews.

The script is a PC-side lab for comparing palette and dithering strategies before
porting the chosen path back to Arduino.
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass
from pathlib import Path

import numpy as np
from PIL import Image, ImageEnhance, ImageFilter, ImageOps


TARGET_W = 800
TARGET_H = 480


@dataclass(frozen=True)
class RgbPalette:
    names: tuple[str, ...]
    screen_rgb: np.ndarray
    match_rgb: np.ndarray
    codes: tuple[int, ...]


OFFICIAL_E6 = RgbPalette(
    names=("white", "green", "red", "yellow", "blue", "black"),
    screen_rgb=np.array(
        [
            (255, 255, 255),
            (29, 185, 84),
            (229, 57, 53),
            (255, 216, 0),
            (0, 76, 255),
            (0, 0, 0),
        ],
        dtype=np.float64,
    ),
    match_rgb=np.array(
        [
            (255, 255, 255),
            (29, 185, 84),
            (229, 57, 53),
            (255, 216, 0),
            (0, 76, 255),
            (0, 0, 0),
        ],
        dtype=np.float64,
    ),
    codes=(0x0, 0x2, 0x6, 0xB, 0xD, 0xF),
)


MEASURED_E6 = RgbPalette(
    names=("black", "white", "yellow", "red", "blue", "green"),
    screen_rgb=np.array(
        [
            (0, 0, 0),
            (255, 255, 255),
            (255, 216, 0),
            (229, 57, 53),
            (0, 76, 255),
            (29, 185, 84),
        ],
        dtype=np.float64,
    ),
    match_rgb=np.array(
        [
            (2, 2, 2),
            (190, 190, 190),
            (205, 202, 0),
            (135, 19, 0),
            (5, 64, 158),
            (39, 102, 60),
        ],
        dtype=np.float64,
    ),
    codes=(0xF, 0x0, 0xB, 0x6, 0xD, 0x2),
)


SENSECRAFT_LIKE_E6 = RgbPalette(
    names=("black", "white", "yellow", "red", "blue", "green"),
    screen_rgb=np.array(
        [
            (10, 10, 10),
            (238, 238, 232),
            (214, 203, 28),
            (142, 34, 22),
            (28, 74, 154),
            (42, 112, 70),
        ],
        dtype=np.float64,
    ),
    match_rgb=np.array(
        [
            (18, 18, 18),
            (215, 215, 205),
            (210, 194, 24),
            (142, 34, 22),
            (30, 72, 150),
            (45, 108, 72),
        ],
        dtype=np.float64,
    ),
    codes=(0xF, 0x0, 0xB, 0x6, 0xD, 0x2),
)


BAYER8 = np.array(
    [
        0, 48, 12, 60, 3, 51, 15, 63,
        32, 16, 44, 28, 35, 19, 47, 31,
        8, 56, 4, 52, 11, 59, 7, 55,
        40, 24, 36, 20, 43, 27, 39, 23,
        2, 50, 14, 62, 1, 49, 13, 61,
        34, 18, 46, 30, 33, 17, 45, 29,
        10, 58, 6, 54, 9, 57, 5, 53,
        42, 26, 38, 22, 41, 25, 37, 21,
    ],
    dtype=np.float64,
).reshape((8, 8))


JARVIS_TAPS = (
    (1, 0, 7 / 48), (2, 0, 5 / 48),
    (-2, 1, 3 / 48), (-1, 1, 5 / 48), (0, 1, 7 / 48),
    (1, 1, 5 / 48), (2, 1, 3 / 48),
    (-2, 2, 1 / 48), (-1, 2, 3 / 48), (0, 2, 5 / 48),
    (1, 2, 3 / 48), (2, 2, 1 / 48),
)


FS_TAPS = (
    (1, 0, 7 / 16),
    (-1, 1, 3 / 16),
    (0, 1, 5 / 16),
    (1, 1, 1 / 16),
)


def prepare_image(path: Path, width: int, height: int) -> Image.Image:
    img = Image.open(path).convert("RGB")
    img = ImageOps.exif_transpose(img)
    if img.height > img.width:
        img = img.rotate(-90, expand=True)
    src_ratio = img.width / img.height
    dst_ratio = width / height
    if src_ratio > dst_ratio:
        new_h = height
        new_w = round(height * src_ratio)
    else:
        new_w = width
        new_h = round(width / src_ratio)
    img = img.resize((new_w, new_h), Image.Resampling.LANCZOS)
    left = (new_w - width) // 2
    top = (new_h - height) // 2
    return img.crop((left, top, left + width, top + height))


def rgb_to_lab(rgb: np.ndarray) -> np.ndarray:
    rgb01 = rgb / 255.0
    mask = rgb01 <= 0.04045
    linear = np.where(mask, rgb01 / 12.92, ((rgb01 + 0.055) / 1.055) ** 2.4)
    x = linear[..., 0] * 0.4124564 + linear[..., 1] * 0.3575761 + linear[..., 2] * 0.1804375
    y = linear[..., 0] * 0.2126729 + linear[..., 1] * 0.7151522 + linear[..., 2] * 0.0721750
    z = linear[..., 0] * 0.0193339 + linear[..., 1] * 0.1191920 + linear[..., 2] * 0.9503041
    xyz = np.stack((x / 0.95047, y / 1.0, z / 1.08883), axis=-1)
    eps = 216 / 24389
    kappa = 24389 / 27
    f = np.where(xyz > eps, np.cbrt(xyz), (kappa * xyz + 16) / 116)
    lab = np.empty_like(f)
    lab[..., 0] = 116 * f[..., 1] - 16
    lab[..., 1] = 500 * (f[..., 0] - f[..., 1])
    lab[..., 2] = 200 * (f[..., 1] - f[..., 2])
    return lab


def apply_script_adjustments(img: Image.Image) -> Image.Image:
    out = ImageEnhance.Brightness(img).enhance(1.03)
    out = ImageEnhance.Color(out).enhance(1.30)
    out = apply_vibrance(out, 0.20)
    out = out.filter(ImageFilter.UnsharpMask(radius=6, percent=35, threshold=2))
    return apply_scurve(out, 0.90, 0.40, 1.60, 0.47)


def apply_vibrance(img: Image.Image, amount: float) -> Image.Image:
    arr = np.asarray(img, dtype=np.float64) / 255.0
    maxc = arr.max(axis=2)
    minc = arr.min(axis=2)
    sat = maxc - minc
    protect = np.ones_like(sat)
    red = (arr[..., 0] == maxc) & (sat > 1e-6)
    green = (arr[..., 1] == maxc) & (sat > 1e-6)
    blue = (arr[..., 2] == maxc) & (sat > 1e-6)
    hue = np.zeros_like(sat)
    hue[red] = ((arr[..., 1][red] - arr[..., 2][red]) / sat[red]) % 6
    hue[green] = (arr[..., 2][green] - arr[..., 0][green]) / sat[green] + 2
    hue[blue] = (arr[..., 0][blue] - arr[..., 1][blue]) / sat[blue] + 4
    hue_deg = hue * 60
    protect[(hue_deg <= 50) | (hue_deg >= 330)] = 0.45
    boost = amount * (1.0 - sat) * protect
    new_sat = np.clip(sat + boost * sat, 0.0, 1.0)
    scale = np.divide(new_sat, sat, out=np.ones_like(sat), where=sat > 1e-6)
    out = maxc[..., None] - (maxc[..., None] - arr) * scale[..., None]
    return Image.fromarray(np.clip(out * 255, 0, 255).astype(np.uint8), "RGB")


def apply_scurve(img: Image.Image, strength: float, shadow: float, highlight: float, midpoint: float) -> Image.Image:
    arr = np.asarray(img, dtype=np.float64) / 255.0
    out = arr.copy()
    low = out <= midpoint
    low_exp = max(0.05, 1.0 - strength * shadow)
    high_exp = max(0.05, 1.0 + strength * highlight)
    out[low] = ((out[low] / midpoint) ** low_exp) * midpoint
    out[~low] = midpoint + (((out[~low] - midpoint) / (1.0 - midpoint)) ** high_exp) * (1.0 - midpoint)
    return Image.fromarray(np.clip(out * 255, 0, 255).astype(np.uint8), "RGB")


def nearest_rgb(pixel: np.ndarray, palette: RgbPalette) -> int:
    diff = palette.match_rgb - pixel
    dist = np.sum(diff * diff, axis=1)
    return int(np.argmin(dist))


def nearest_lab(pixel_lab: np.ndarray, pixel_rgb: np.ndarray, palette_lab: np.ndarray) -> int:
    diff = palette_lab - pixel_lab
    dist = diff[:, 0] * diff[:, 0] * 0.70 + diff[:, 1] * diff[:, 1] + diff[:, 2] * diff[:, 2]
    r, g, b = pixel_rgb
    maxc = max(r, g, b)
    minc = min(r, g, b)
    chroma = maxc - minc
    luma = 0.2126 * r + 0.7152 * g + 0.0722 * b

    if luma < 70 and chroma < 55:
        dist[2:] += 2600
    if luma < 45:
        dist[4] += 3400
        dist[5] += 4200
    if chroma < 24:
        dist[2:] += 1800
    if g > r + 10 and g > b + 10:
        dist[5] *= 0.72
    if b > r + 12 and b > g + 8:
        dist[4] *= 0.82
    if r > g + 12 and r > b + 12:
        dist[3] *= 0.82
    if r > 120 and g > 100 and b < 90:
        dist[2] *= 0.76
    return int(np.argmin(dist))


def dither_rgb(img: Image.Image, palette: RgbPalette, taps: tuple[tuple[int, int, float], ...], strength: float) -> Image.Image:
    src = np.asarray(img, dtype=np.float64).copy()
    h, w, _ = src.shape
    out = np.zeros_like(src)
    for y in range(h):
        for x in range(w):
            old = np.clip(src[y, x], 0, 255)
            idx = nearest_rgb(old, palette)
            out[y, x] = palette.screen_rgb[idx]
            err = (old - palette.match_rgb[idx]) * strength
            for dx, dy, weight in taps:
                nx = x + dx
                ny = y + dy
                if 0 <= nx < w and 0 <= ny < h:
                    src[ny, nx] = np.clip(src[ny, nx] + err * weight, 0, 255)
    return Image.fromarray(out.astype(np.uint8), "RGB")


def dither_lab(img: Image.Image, palette: RgbPalette, strength: float) -> Image.Image:
    src_rgb = np.asarray(img, dtype=np.float64).copy()
    src_lab = rgb_to_lab(src_rgb)
    palette_lab = rgb_to_lab(palette.match_rgb.reshape((1, -1, 3))).reshape((-1, 3))
    h, w, _ = src_rgb.shape
    out_rgb = np.zeros_like(src_rgb)
    for y in range(h):
        left_to_right = (y % 2) == 0
        xs = range(w) if left_to_right else range(w - 1, -1, -1)
        direction = 1 if left_to_right else -1
        for x in xs:
            old_lab = src_lab[y, x]
            old_rgb = np.clip(src_rgb[y, x], 0, 255)
            idx = nearest_lab(old_lab, old_rgb, palette_lab)
            out_rgb[y, x] = palette.screen_rgb[idx]
            err_lab = (old_lab - palette_lab[idx]) * strength
            err_rgb = (old_rgb - palette.match_rgb[idx]) * (strength * 0.20)
            for dx, dy, weight in JARVIS_TAPS:
                nx = x + dx * direction
                ny = y + dy
                if 0 <= nx < w and 0 <= ny < h:
                    src_lab[ny, nx] += err_lab * weight
                    src_rgb[ny, nx] = np.clip(src_rgb[ny, nx] + err_rgb * weight, 0, 255)
    return Image.fromarray(np.clip(out_rgb, 0, 255).astype(np.uint8), "RGB")


def ordered_lab(img: Image.Image, palette: RgbPalette, spread: float) -> Image.Image:
    rgb = np.asarray(img, dtype=np.float64)
    lab = rgb_to_lab(rgb)
    palette_lab = rgb_to_lab(palette.match_rgb.reshape((1, -1, 3))).reshape((-1, 3))
    h, w, _ = rgb.shape
    out = np.zeros_like(rgb)
    for y in range(h):
        for x in range(w):
            mod = (BAYER8[y & 7, x & 7] / 63.0 - 0.5) * spread
            probe = lab[y, x].copy()
            probe[0] = np.clip(probe[0] + mod, 0, 100)
            idx = nearest_lab(probe, rgb[y, x], palette_lab)
            out[y, x] = palette.screen_rgb[idx]
    return Image.fromarray(np.clip(out, 0, 255).astype(np.uint8), "RGB")


def cartoon_halftone_hsv(img: Image.Image) -> Image.Image:
    rgb = np.asarray(img, dtype=np.float64)
    h, w, _ = rgb.shape
    out = np.zeros_like(rgb)
    bayer = BAYER8 / 63.0

    for y in range(h):
        for x in range(w):
            r, g, b = rgb[y, x]
            rn, gn, bn = r / 255.0, g / 255.0, b / 255.0
            maxc = max(rn, gn, bn)
            minc = min(rn, gn, bn)
            chroma = maxc - minc
            sat = 0.0 if maxc <= 1e-6 else chroma / maxc
            luma = (0.2126 * r + 0.7152 * g + 0.0722 * b) / 255.0

            if chroma <= 1e-6:
                hue = 0.0
            elif maxc == rn:
                hue = ((gn - bn) / chroma) % 6.0 * 60.0
            elif maxc == gn:
                hue = ((bn - rn) / chroma + 2.0) * 60.0
            else:
                hue = ((rn - gn) / chroma + 4.0) * 60.0

            t = bayer[y & 7, x & 7]
            color = np.array((238, 238, 232), dtype=np.float64)

            if luma < 0.18:
                color = np.array((10, 10, 10), dtype=np.float64)
            elif luma < 0.34 and sat < 0.45:
                black_amount = np.clip((0.42 - luma) / 0.24, 0.0, 1.0)
                color = np.array((10, 10, 10), dtype=np.float64) if t < black_amount else color
            elif 38.0 <= hue <= 78.0 and sat > 0.12:
                yellow_amount = np.clip((sat * 1.15 + (0.86 - luma) * 0.45), 0.12, 1.0)
                color = np.array((214, 203, 28), dtype=np.float64) if t < yellow_amount else color
            elif (hue <= 22.0 or hue >= 342.0) and sat > 0.10:
                red_amount = np.clip(sat * 0.95 + (0.74 - luma) * 0.28, 0.10, 0.95)
                color = np.array((142, 34, 22), dtype=np.float64) if t < red_amount else color
            elif 22.0 < hue < 38.0 and sat > 0.15:
                red_amount = np.clip(sat * 0.42 + (0.72 - luma) * 0.18, 0.0, 0.55)
                yellow_amount = np.clip(sat * 0.56, 0.10, 0.72)
                if t < red_amount:
                    color = np.array((142, 34, 22), dtype=np.float64)
                elif t < red_amount + yellow_amount:
                    color = np.array((214, 203, 28), dtype=np.float64)
            elif 165.0 <= hue <= 245.0 and sat > 0.08:
                blue_amount = np.clip(sat * 0.82 + (0.70 - luma) * 0.18, 0.08, 0.82)
                green_amount = np.clip((1.0 - abs(hue - 175.0) / 75.0) * sat * 0.38, 0.0, 0.42)
                if t < green_amount:
                    color = np.array((42, 112, 70), dtype=np.float64)
                elif t < green_amount + blue_amount:
                    color = np.array((28, 74, 154), dtype=np.float64)
            elif 78.0 < hue < 165.0 and sat > 0.08:
                green_amount = np.clip(sat * 0.78 + (0.68 - luma) * 0.14, 0.06, 0.78)
                color = np.array((42, 112, 70), dtype=np.float64) if t < green_amount else color

            if 0.18 <= luma <= 0.72 and sat < 0.22 and t < np.clip((0.72 - luma) * 0.38, 0.0, 0.22):
                color = np.array((10, 10, 10), dtype=np.float64)

            out[y, x] = color

    return Image.fromarray(np.clip(out, 0, 255).astype(np.uint8), "RGB")


def save_index_bmp(img: Image.Image, palette: RgbPalette, output: Path) -> None:
    rgb = np.asarray(img, dtype=np.float64)
    h, w, _ = rgb.shape
    index = Image.new("P", (w, h))
    pal = []
    for color in palette.screen_rgb.astype(np.uint8):
        pal.extend(color.tolist())
    pal.extend([0] * (768 - len(pal)))
    index.putpalette(pal)
    pixels = np.zeros((h, w), dtype=np.uint8)
    for y in range(h):
        for x in range(w):
            diff = palette.screen_rgb - rgb[y, x]
            pixels[y, x] = int(np.argmin(np.sum(diff * diff, axis=1)))
    index.putdata(pixels.reshape(-1).tolist())
    index.save(output, "BMP")


def make_contact_sheet(items: list[tuple[str, Image.Image]], output: Path) -> None:
    thumb_w = 400
    thumb_h = 240
    label_h = 34
    cols = 2
    rows = (len(items) + cols - 1) // cols
    sheet = Image.new("RGB", (cols * thumb_w, rows * (thumb_h + label_h)), "white")
    for i, (label, img) in enumerate(items):
        x = (i % cols) * thumb_w
        y = (i // cols) * (thumb_h + label_h)
        sheet.paste(img.resize((thumb_w, thumb_h), Image.Resampling.NEAREST), (x, y + label_h))
        draw_label(sheet, label, x + 8, y + 8)
    sheet.save(output)


def draw_label(img: Image.Image, label: str, x: int, y: int) -> None:
    from PIL import ImageDraw

    draw = ImageDraw.Draw(img)
    draw.text((x, y), label, fill=(0, 0, 0))


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Generate E1002 6-color comparison previews.")
    parser.add_argument("input", type=Path, help="Input JPG/PNG image.")
    parser.add_argument("--out", type=Path, default=Path("/private/tmp/e1002_color_compare"), help="Output folder.")
    parser.add_argument("--width", type=int, default=TARGET_W)
    parser.add_argument("--height", type=int, default=TARGET_H)
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    args.out.mkdir(parents=True, exist_ok=True)

    prepared = prepare_image(args.input, args.width, args.height)
    official = dither_rgb(prepared, OFFICIAL_E6, FS_TAPS, 1.0)
    measured = dither_rgb(apply_script_adjustments(prepared), MEASURED_E6, FS_TAPS, 1.0)
    sense_ordered = ordered_lab(apply_script_adjustments(prepared), SENSECRAFT_LIKE_E6, 15.0)
    sense_lab = dither_lab(apply_script_adjustments(prepared), SENSECRAFT_LIKE_E6, 0.82)
    cartoon = cartoon_halftone_hsv(prepared)

    outputs = [
        ("00_prepared.png", prepared),
        ("01_official_fs_rgb.png", official),
        ("02_measured_script_fs.png", measured),
        ("03_sensecraft_like_ordered_lab.png", sense_ordered),
        ("04_sensecraft_like_jarvis_lab.png", sense_lab),
        ("05_cartoon_halftone_hsv.png", cartoon),
    ]
    for name, image in outputs:
        image.save(args.out / name)
    save_index_bmp(sense_lab, SENSECRAFT_LIKE_E6, args.out / "04_sensecraft_like_jarvis_lab_indexed.bmp")
    save_index_bmp(cartoon, SENSECRAFT_LIKE_E6, args.out / "05_cartoon_halftone_hsv_indexed.bmp")
    make_contact_sheet(outputs, args.out / "contact_sheet.png")
    print(f"Output: {args.out}")


if __name__ == "__main__":
    main()
