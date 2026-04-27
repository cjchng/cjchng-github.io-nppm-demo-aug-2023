#!/usr/bin/env python3
from __future__ import annotations

import argparse
from pathlib import Path


SKIP_PREFIXES = ("#", "*", "{", "}", "$")


def normalize_path_token(value: str) -> str:
    if value.startswith("@DROPBOX@/"):
        return "@DROPBOX_ROOT@/" + value[len("@DROPBOX@/") :]
    return value


def read_relevant_lines(path: Path) -> list[str]:
    lines: list[str] = []
    for raw in path.read_text(encoding="utf-8").splitlines():
        line = raw.strip()
        if not line:
            continue
        if line.startswith(SKIP_PREFIXES):
            continue
        lines.append(line)
    return lines


class CastParser:
    def __init__(self, lines: list[str]) -> None:
        self.lines = lines
        self.index = 0

    def next_line(self) -> str:
        if self.index >= len(self.lines):
            raise ValueError("Unexpected end of recipe while parsing.")
        line = self.lines[self.index]
        self.index += 1
        return line

    def expect(self, token: str) -> str:
        actual = self.next_line()
        if actual != token:
            raise ValueError(f"Expected token {token!r}, found {actual!r}")
        return self.next_line()


def parse_recipe(path: Path) -> dict:
    p = CastParser(read_relevant_lines(path))

    cast_version = p.expect("@cast_version")
    camera_name = p.expect("@md->cfg->cameraName")

    camera_values = [float(v) for v in p.next_line().split()]
    if len(camera_values) != 14:
        raise ValueError(f"Expected 14 camera values, found {len(camera_values)}")

    alpha_range_token = p.next_line()
    if alpha_range_token not in {"@alpha_range_vendor", "@alpha_range_vender"}:
        raise ValueError(f"Expected @alpha_range_vendor or @alpha_range_vender, found {alpha_range_token!r}")
    alpha_range_vendor = float(p.next_line())

    alpha_pano_min = float(p.expect("@alpha_pano_min"))
    alpha_pano_max = float(p.expect("@alpha_pano_max"))
    beta_pano_begin = float(p.expect("@beta_pano_begin"))
    beta_pano_range = float(p.expect("@beta_pano_range"))
    alpha_pano_min_init = float(p.expect("@alpha_pano_min_init"))
    alpha_pano_max_init = float(p.expect("@alpha_pano_max_init"))
    alpha_pipe_init = float(p.expect("@alpha_pipe_init"))
    beta_pipe_init = float(p.expect("@beta_pipe_init"))
    zoom_pipe_init = float(p.expect("@zoom_pipe_init"))
    pitch_car_init = float(p.expect("@pitch_car_init"))
    yaw_car_init = float(p.expect("@yaw_car_init"))
    zoom_car_init = float(p.expect("@zoom_car_init"))
    virtual_origin_left = int(float(p.expect("@VIRTUAL_ORGIN_LEFT")))
    virtual_origin_top = int(float(p.expect("@VIRTUAL_ORGIN_TOP")))
    working_root = normalize_path_token(p.expect("@la_nppm_ing_abs_folder"))
    camera_folder = p.expect("@la_camera_name_as_inst_short_folder")
    image_folder = p.expect("@la_using_image_short_folder")
    image_file = p.expect("@la_image_inst_short_file")
    projection_mode = p.expect("@la_car_pano_or_tube_tube_pano")
    mapper_folder = p.expect("@la_mappers_folder_short")
    title = p.expect("@la_screen_title")
    display_cols = int(float(p.expect("@la_display_cols")))
    display_rows = int(float(p.expect("@la_display_rows")))
    frame_aspect = float(p.expect("@la_vir_frame_asp"))
    media_mode = p.expect("@la_is_camera_ready")
    usb_camera = p.expect("@la_is_usb_camera").lower() in {"1", "true", "yes"}
    media_source = normalize_path_token(p.expect("@la_video_stream_uri"))

    pyz = []
    for _ in range(6):
        row = []
        for _ in range(3):
            p.expect("@pyz[a_row][a_col]")
            row.append(float(p.lines[p.index - 1]))
        pyz.append(row)

    ic_alpha_degree = float(p.expect("@la_iC_alpha_degree"))
    ic_beta_degree = float(p.expect("@la_iC_beta_degree"))
    roi_left = float(p.expect("@la_roi_car_pano_left"))
    roi_top = float(p.expect("@la_roi_car_pano_top"))
    roi_cols = float(p.expect("@la_roi_car_pano_cols"))
    roi_rows = float(p.expect("@la_roi_car_pano_rows"))
    display_aspect = float(p.expect("@la_display_asp"))
    rt_alpha_from = float(p.expect("@la_rt_alpha_from"))
    rt_alpha_to = float(p.expect("@la_rt_alpha_to"))

    if media_mode == "image":
        media_type = "image"
    elif media_mode == "video":
        media_type = "stream_http" if "://" in media_source or media_source.startswith("//") else "video_file"
    elif media_mode == "camera" and usb_camera:
        media_type = "camera_usb"
    else:
        media_type = "unsupported_legacy_rpi"

    return {
        "cast_version": cast_version,
        "camera_name": camera_name,
        "camera_values": camera_values,
        "alpha_range_vendor": alpha_range_vendor,
        "alpha_pano_min": alpha_pano_min,
        "alpha_pano_max": alpha_pano_max,
        "beta_pano_begin": beta_pano_begin,
        "beta_pano_range": beta_pano_range,
        "alpha_pano_min_init": alpha_pano_min_init,
        "alpha_pano_max_init": alpha_pano_max_init,
        "alpha_pipe_init": alpha_pipe_init,
        "beta_pipe_init": beta_pipe_init,
        "zoom_pipe_init": zoom_pipe_init,
        "pitch_car_init": pitch_car_init,
        "yaw_car_init": yaw_car_init,
        "zoom_car_init": zoom_car_init,
        "virtual_origin_left": virtual_origin_left,
        "virtual_origin_top": virtual_origin_top,
        "working_root": working_root,
        "camera_folder": camera_folder,
        "image_folder": image_folder,
        "image_file": image_file,
        "projection_mode": projection_mode,
        "mapper_folder": mapper_folder,
        "title": title,
        "display_cols": display_cols,
        "display_rows": display_rows,
        "frame_aspect": frame_aspect,
        "media_type": media_type,
        "usb_camera": usb_camera,
        "media_source": media_source,
        "pyz": pyz,
        "ic_alpha_degree": ic_alpha_degree,
        "ic_beta_degree": ic_beta_degree,
        "roi_left": roi_left,
        "roi_top": roi_top,
        "roi_cols": roi_cols,
        "roi_rows": roi_rows,
        "display_aspect": display_aspect,
        "rt_alpha_from": rt_alpha_from,
        "rt_alpha_to": rt_alpha_to,
    }


def fmt_scalar(value) -> str:
    if isinstance(value, bool):
        return "true" if value else "false"
    if isinstance(value, int):
        return str(value)
    if isinstance(value, float):
        return f"{value:g}"
    return f'"{value}"'


def render_yaml(data: dict) -> str:
    camera = data["camera_values"]
    lines = [
        "version: 1",
        "",
        "camera:",
        f"  name: {fmt_scalar(data['camera_name'])}",
        "  sensor:",
        f"    width: {fmt_scalar(camera[0])}",
        f"    height: {fmt_scalar(camera[1])}",
        "  intrinsics:",
        f"    icx: {fmt_scalar(camera[2])}",
        f"    icy: {fmt_scalar(camera[3])}",
        f"    ratio: {fmt_scalar(camera[4])}",
        f"    image_width: {fmt_scalar(camera[5])}",
        f"    image_height: {fmt_scalar(camera[6])}",
        f"    calibration_ratio: {fmt_scalar(camera[7])}",
        "    para: [" + ", ".join(fmt_scalar(v) for v in camera[8:14]) + "]",
        "",
        "paths:",
        f"  working_root: {fmt_scalar(data['working_root'])}",
        f"  camera_folder: {fmt_scalar(data['camera_folder'])}",
        f"  image_folder: {fmt_scalar(data['image_folder'])}",
        f"  image_file: {fmt_scalar(data['image_file'])}",
        f"  mapper_folder: {fmt_scalar(data['mapper_folder'])}",
        "",
        "media:",
        f"  type: {fmt_scalar(data['media_type'])}",
        f"  usb_camera: {fmt_scalar(data['usb_camera'])}",
        f"  source: {fmt_scalar(data['media_source'])}",
        "",
        "display:",
        f"  title: {fmt_scalar(data['title'])}",
        f"  cols: {fmt_scalar(data['display_cols'])}",
        f"  rows: {fmt_scalar(data['display_rows'])}",
        f"  frame_aspect: {fmt_scalar(data['frame_aspect'])}",
        f"  virtual_origin_left: {fmt_scalar(data['virtual_origin_left'])}",
        f"  virtual_origin_top: {fmt_scalar(data['virtual_origin_top'])}",
        "",
        "projection:",
        f"  mode: {fmt_scalar(data['projection_mode'])}",
        f"  ic_alpha_degree: {fmt_scalar(data['ic_alpha_degree'])}",
        f"  ic_beta_degree: {fmt_scalar(data['ic_beta_degree'])}",
        "",
        "views:",
        "  pyz:",
    ]
    for row in data["pyz"]:
        lines.append("    - [" + ", ".join(fmt_scalar(v) for v in row) + "]")
    lines += [
        "",
        "roi:",
        f"  car_pano_left: {fmt_scalar(data['roi_left'])}",
        f"  car_pano_top: {fmt_scalar(data['roi_top'])}",
        f"  car_pano_cols: {fmt_scalar(data['roi_cols'])}",
        f"  car_pano_rows: {fmt_scalar(data['roi_rows'])}",
        f"  display_aspect: {fmt_scalar(data['display_aspect'])}",
        f"  rt_alpha_from: {fmt_scalar(data['rt_alpha_from'])}",
        f"  rt_alpha_to: {fmt_scalar(data['rt_alpha_to'])}",
        "",
        "tuning:",
        f"  alpha_range_vendor: {fmt_scalar(data['alpha_range_vendor'])}",
        f"  alpha_pano_min: {fmt_scalar(data['alpha_pano_min'])}",
        f"  alpha_pano_max: {fmt_scalar(data['alpha_pano_max'])}",
        f"  beta_pano_begin: {fmt_scalar(data['beta_pano_begin'])}",
        f"  beta_pano_range: {fmt_scalar(data['beta_pano_range'])}",
        f"  alpha_pano_min_init: {fmt_scalar(data['alpha_pano_min_init'])}",
        f"  alpha_pano_max_init: {fmt_scalar(data['alpha_pano_max_init'])}",
        f"  alpha_pipe_init: {fmt_scalar(data['alpha_pipe_init'])}",
        f"  beta_pipe_init: {fmt_scalar(data['beta_pipe_init'])}",
        f"  zoom_pipe_init: {fmt_scalar(data['zoom_pipe_init'])}",
        f"  pitch_car_init: {fmt_scalar(data['pitch_car_init'])}",
        f"  yaw_car_init: {fmt_scalar(data['yaw_car_init'])}",
        f"  zoom_car_init: {fmt_scalar(data['zoom_car_init'])}",
        "",
    ]
    return "\n".join(lines)


def default_output_path(input_path: Path) -> Path:
    return input_path.with_suffix(".yaml")


def main() -> int:
    parser = argparse.ArgumentParser(description="Convert legacy .cast/.script recipes to YAML v1.")
    parser.add_argument("input", type=Path, help="Input .cast or .script recipe")
    parser.add_argument("output", type=Path, nargs="?", help="Output .yaml recipe path")
    args = parser.parse_args()

    if not args.input.exists():
        parser.error(f"Input file does not exist: {args.input}")

    output_path = args.output or default_output_path(args.input)
    parsed = parse_recipe(args.input)
    output_path.write_text(render_yaml(parsed), encoding="utf-8")
    print(f"Converted {args.input} -> {output_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
