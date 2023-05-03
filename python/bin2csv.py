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

def velocity(accel, time):
    accel_ms = accel * 9.80665
    return accel_ms * time

# TODO: Replace above with below
# def velocity(s, u, a, t):
#     """
#     Calculates velocity using the SUVAT equation v = u + at,
#     where s is the displacement, u is the initial velocity, 
#     a is the acceleration, and t is the time taken.
#     """
#     accel_ms = a * 9.80665
#     v = u + accel_ms*t
#     return v



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
deltatime = 0
counter = 0
Velocity = 0

with open(args.ifile, 'rb') as f:
    with open(args.ofile, 'w') as csvfile:
        csvwriter = csv.writer(csvfile)
        csvwriter.writerow(["Time (ms)", "Accelerometer X (g)", "Accelerometer Y (g)", "Accelerometer Z (g)",
                           "Temperature (ºC)", "Pressure (hPa)", "Altitude (m)", "Velocity (m/s)", "Magnitude (g)"])
        while True:
            data = f.read(struct_len)
            if not data:
                break
            if len(data) != struct_len:
                continue
            s = struct_unpack(data)
            if deltatime == 0:
                deltatime = s[0]
            Magnitude = magnitude(s[1], s[2], s[3])
            Altitude = altitude(s[4], s[5])
            if counter % 10 == 0:
                Velocity = velocity(Magnitude, s[0]-deltatime)
                deltatime = s[0]
            s = s + (Altitude, Velocity , Magnitude)
            counter += 1
            csvwriter.writerow(s)
print("Done!")
