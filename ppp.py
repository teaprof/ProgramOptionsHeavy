import argparse

parser = argparse.ArgumentParser()
parser.add_argument("-f", nargs="*")
parser.add_argument("bar")
parser.add_argument("-q")

args = parser.parse_args()
print(args)
