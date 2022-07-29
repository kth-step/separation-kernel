import statistics
import subprocess
import time

# Config #
is_qemu = False
num_rounds = 1
discard_first_n = 2
output_file = "./results/benchmark_results.txt"

# Global variables #
output = ""
discard_string = ""
values = []

# Execute benchmark #
start_time = time.time()
print("Begin benchmark script")
for round in range(num_rounds):
    round_values = []
    hart = 0
    if is_qemu:
        p = subprocess.Popen(["make", "--no-print-directory", "noninteractive-qemu"], stdout=subprocess.PIPE, universal_newlines=True)
    else:
        p_openocd = subprocess.Popen(["./remote/openocd.sh"], stdout=subprocess.DEVNULL)
        p = subprocess.Popen(["./remote/uart.sh"], stdout=subprocess.PIPE, universal_newlines=True)
        p_board = subprocess.Popen(["make", "--no-print-directory", "debug-hifive-unleashed"], 
            stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)

    line = p.stdout.readline()
    while not "DONE" in line:
        if line[0:17] == "Values from hart ":
            # We assume we never have more than 9 harts
            hart = int(line[17])
            if len(values) < (hart + 1):
                values.append([])
        elif "End of values from hart " in line:
            if discard_first_n > 0:
                first_values = round_values[0:discard_first_n]
                round_values = round_values[discard_first_n:]
                discard_string += "Discarding first " + str(discard_first_n) + " value(s) from hart " + str(hart) + " (might have been bad due to being in boot round or similar). Discarded value(s): " 
                for i in range(discard_first_n):
                    discard_string += str(first_values[i]) + " "
                discard_string += "\n"
            values[hart].extend(round_values)
            round_values = []
        elif line[0:6] == "Value=":
            round_values.append(int(line[6:]))
        elif round == 0 and line != "\n":
            output += line
        line = p.stdout.readline()
    print("Round", str(round+1), "done")
    if not is_qemu:
        p_openocd.kill()
        p_board.kill()
    p.kill()
    if is_qemu:
        subprocess.run(["make", "--no-print-directory", "stop-qemu"])
    else:
        subprocess.run(["make", "--no-print-directory", "stop-debugging"])
        subprocess.run(["./remote/cleanup.sh"])

    output += "\n"
    

if not is_qemu:
    subprocess.run("reset")

ex_time = time.time()
print("Execution time:", ex_time - start_time)
output += "Time since script started until benchmark execution was done: " + str(ex_time - start_time)
output += "\nIn minutes: " + str((ex_time - start_time)/60) + "\n\n"

# Generate remaining output #
for hart, hart_vals in enumerate(values):
    formatting_style = "{:,.0f}"
    var_str = formatting_style.format(statistics.variance(hart_vals))
    entries_str = formatting_style.format(len(hart_vals))
    max_number_length = max(len(var_str), len(entries_str))

    output += "Hart " + str(hart) + ":\n"
    output += "(The following values are formatted and rounded to not include fractions)\n"
    output += "Number of entries: ".ljust(30, ' ') + entries_str.rjust(max_number_length, ' ') + "\n"
    output += "Average: ".ljust(30, ' ') + formatting_style.format(statistics.mean(hart_vals)).rjust(max_number_length, ' ') + "\n"
    output += "Standard deviation: ".ljust(30, ' ') + formatting_style.format(statistics.stdev(hart_vals)).rjust(max_number_length, ' ') + "\n" # This calculates the standard deviation of a sample (pstdev for standard deviation of population)
    output += "Variance: ".ljust(30, ' ') + var_str.rjust(max_number_length, ' ') + "\n" # This calculates the variance of a sample (pvariance for variance of population)
    output += "Max: ".ljust(30, ' ') + formatting_style.format(max(hart_vals)).rjust(max_number_length, ' ') + "\n"
    output += "Min: ".ljust(30, ' ') + formatting_style.format(min(hart_vals)).rjust(max_number_length, ' ') + "\n"
    output += "\n"

output += discard_string
output += "\n"

output += "Original raw values:\n"
for hart, hart_vals in enumerate(values):
    output += "Hart " + str(hart) + ":\n"
    for i in range(len(hart_vals)):
        output += str(hart_vals[i]) + "\n"

# Write output to file and end script #
with open(output_file, "w") as f:
    f.write(output)
print("Benchmark script finished")
end_time = time.time()
print("Total time:", end_time - start_time, ", in minutes:", (end_time - start_time)/60)