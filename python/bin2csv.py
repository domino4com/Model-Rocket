#!/usr/bin/env python3
import math
import struct
import csv
import argparse
import os.path


def is_valid_file(parser, arg):
    if not os.path.exists(arg):
        parser.error("The file %s does not exist!" % arg)
    else:
        return arg


def altitude(temp, pressure):
    R = 8.31432
    g = 9.80665
    M = 0.0289644
    T0 = 288.15
    P0 = 101325
    L = 0.0065
    h = ((T0 / (T0 + L * (temp - 273.15))) **
         (g * M / (R * L))) * (pressure / P0)
    h = (1 - h) ** (R * L / (g * M))
    h = (T0 / L) * (1 - h)
    return h


def magnitude(x, y, z):
    return math.sqrt(x*x + y*y + z*z)

argParser = argparse.ArgumentParser(
    description="Converts Rocket Binary Data to CSV file!")
argParser.add_argument("-i", "--ifile", help="Input Binary File", required=True,
                       dest="ifile", metavar="FILE", type=lambda x: is_valid_file(argParser, x))
argParser.add_argument("-o", "--ofile", help="Output CSV File",
                       required=True, dest="ofile", metavar="FILE")
args = argParser.parse_args()

struct_fmt = '=lfffff'  # long, float, float, float, float, float,
struct_len = struct.calcsize(struct_fmt)
struct_unpack = struct.Struct(struct_fmt).unpack_from
counter = 0

with open(args.ifile, 'rb') as f:
    with open(args.ofile, 'w') as csvfile:
        csvwriter = csv.writer(csvfile)
        csvwriter.writerow(["Time (ms)", "Accelerometer X (g)", "Accelerometer Y (g)", "Accelerometer Z (g)",
                           "Temperature (ºC)", "Pressure (hPa)", "Altitude (m)", "Magnitude (g)"])
        while True:
            data = f.read(struct_len)
            if not data:
                break
            if len(data) != struct_len:
                continue
            s = struct_unpack(data)
            Magnitude = magnitude(s[1], s[2], s[3])
            Altitude = altitude(s[4], s[5])
            s = s + (Altitude,  Magnitude)
            counter += 1
            csvwriter.writerow(s)
print("Done!")
