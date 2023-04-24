import sys
import random as rng

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Requires an input file and an output file")
        exit(1)

    with open(sys.argv[1], "r", encoding="utf8") as f:
        with open(sys.argv[2], "w", encoding="utf8") as of:
            for line in f:
                prio = 0
                new_line = line[:-1] + " " + str(prio) + "\n"
                of.write(new_line)
