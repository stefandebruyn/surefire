fname = "BoardOptions.cmake"

with open(fname, "r") as inp:
    lines = []
    found_board = False
    for line in inp:
        if not found_board:
            if "Mega 2560" in line:
                line = line.replace("# ", "", 1)
                found_board = True
            lines.append(line)
        else:
            lines.append(line)
    assert found_board

with open(fname, "w") as inp:
    inp.writelines(lines)
