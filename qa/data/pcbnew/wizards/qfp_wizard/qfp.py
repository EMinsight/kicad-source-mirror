#!/usr/bin/env python3

# Copyright The KiCad Developers
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the “Software”), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import argparse
import sys

from google.protobuf.json_format import MessageToJson

from kipy.board_types import BoardLayer, BoardRectangle, BoardText, Footprint
from kipy.geometry import Vector2
from kipy.util.units import from_mm
from kipy.wizards import (
    WizardContentType,
    WizardInfo,
    WizardMetaInfo,
    WizardParameter,
    WizardParameterCategory,
    WizardParameterDataType,
)

def build_wizard_info() -> WizardInfo:
    info = WizardInfo()
    info.meta = WizardMetaInfo()
    info.meta.identifier = "org.kicad.generators.qfp.qfp"
    info.meta.name = "QFP"
    info.meta.description = "Quad Flat Package (QFP) footprint wizard"
    info.meta.types_generated = [WizardContentType.WCT_FOOTPRINT]
    info.parameters = [
        WizardParameter.create(
            "n",
            "Pad Count",
            WizardParameterCategory.WPC_PADS,
            WizardParameterDataType.WPDT_INTEGER,
            32,
            min_value=4,
            multiple=4,
        ),
        WizardParameter.create(
            "e",
            "Pad Pitch",
            WizardParameterCategory.WPC_PADS,
            WizardParameterDataType.WPDT_DISTANCE,
            from_mm(0.8),
        ),
        WizardParameter.create(
            "X1",
            "Pad Width",
            WizardParameterCategory.WPC_PADS,
            WizardParameterDataType.WPDT_DISTANCE,
            from_mm(0.55),
        ),
        WizardParameter.create(
            "Y1",
            "Pad Length",
            WizardParameterCategory.WPC_PADS,
            WizardParameterDataType.WPDT_DISTANCE,
            from_mm(1.5),
        ),
        WizardParameter.create(
            "C1",
            "Horizontal spacing",
            WizardParameterCategory.WPC_PADS,
            WizardParameterDataType.WPDT_DISTANCE,
            from_mm(8.4),
        ),
        WizardParameter.create(
            "C2",
            "Vertical spacing",
            WizardParameterCategory.WPC_PADS,
            WizardParameterDataType.WPDT_DISTANCE,
            from_mm(8.4),
        ),
        WizardParameter.create(
            "oval",
            "Oval Pads",
            WizardParameterCategory.WPC_PADS,
            WizardParameterDataType.WPDT_BOOL,
            True,
        ),
        WizardParameter.create(
            "D1",
            "Overall Width",
            WizardParameterCategory.WPC_PACKAGE,
            WizardParameterDataType.WPDT_DISTANCE,
            from_mm(7),
        ),
        WizardParameter.create(
            "E1",
            "Overall Height",
            WizardParameterCategory.WPC_PACKAGE,
            WizardParameterDataType.WPDT_DISTANCE,
            from_mm(7),
        ),
        WizardParameter.create(
            "courtyard_margin",
            "Courtyard Margin",
            WizardParameterCategory.WPC_PACKAGE,
            WizardParameterDataType.WPDT_DISTANCE,
            from_mm(0.25),
            min_value=from_mm(0.2),
        ),
    ]
    return info


def build_footprint() -> Footprint:
    footprint = Footprint()
    footprint.id.name = "GeneratedQFP"

    courtyard = BoardRectangle()
    courtyard.layer = BoardLayer.BL_F_CrtYd
    courtyard.top_left = Vector2.from_xy_mm(-2.0, 2.0)
    courtyard.bottom_right = Vector2.from_xy_mm(2.0, -2.0)
    footprint.add_item(courtyard)

    refdes = BoardText()
    refdes.layer = BoardLayer.BL_F_SilkS
    refdes.position = Vector2.from_xy_mm(0.0, 0.0)
    refdes.value = "REF**"
    footprint.add_item(refdes)

    return footprint


def parse_args(argv: list[str]):
    parser = argparse.ArgumentParser(description="QFP footprint wizard")
    parser.add_argument(
        "--get-info",
        action="store_true"
    )
    parser.add_argument(
        "--generate",
        action="store_true"
    )
    return parser.parse_args(argv)


if __name__ == "__main__":
    args = parse_args(sys.argv[1:])
    obj = None

    if args.get_info:
        obj = build_wizard_info()
    elif args.generate:
        obj = build_footprint()

    if obj is not None:
        json = MessageToJson(
            obj.proto,
            preserving_proto_field_name=True,
        )
        print(json)
