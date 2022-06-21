import statistics
import subprocess

# 
is_slack = True
is_instrumentation = False
with open("src/config.h","r") as f:
    for line in f.readlines():
        if "#define INSTRUMENTATION_TEST 1" in line:
            is_slack = False
            is_instrumentation = True

# Config #
num_rounds = 1
output_file = "./benchmark_results.txt"

# Global variables #
output = ""
values = []

# Execute benchmark #
for i in range(num_rounds):
    p = subprocess.Popen(["make", "--no-print-directory", "noninteractive-qemu"], stdout=subprocess.PIPE, universal_newlines=True)

    line = p.stdout.readline()
    while not "DONE" in line:
        if line[0:6] == "Value:":
            values.append(int(line[6:]))
        elif not is_instrumentation and line[0:6] == "Value=":
            values.append(int(line[6:]))
        elif i == 0 and line != "\n" and line[0:6] != "Value=" and not "Compiling ELF file:" in line:
            output += line + "\n"
        line = p.stdout.readline()
    print("Round", str(i+1), "done")
    p.kill()
    subprocess.run(["make", "--no-print-directory", "stop-qemu"])

# Generate output #
if is_slack:
    first_entry = values[0]
    values = values[1:]
    output += "Measuring slack time; first entry is discarded since boot scheduling takes unpredictable time. First entry was: " + str(first_entry) + "\n"
elif is_instrumentation:
    output += "Measuring instrumentation time.\n"

formatting_style = "{:,.0f}"
var_str = formatting_style.format(statistics.variance(values))
max_number_length = len(var_str)

output += "(The following values are formatted and rounded to not include fractions)\n"
output += "Number of entries: ".ljust(30, ' ') + formatting_style.format(len(values)).rjust(max_number_length, ' ') + "\n"
output += "Average: ".ljust(30, ' ') + formatting_style.format(statistics.mean(values)).rjust(max_number_length, ' ') + "\n"
output += "Standard deviation: ".ljust(30, ' ') + formatting_style.format(statistics.stdev(values)).rjust(max_number_length, ' ') + "\n" # This calculates the standard deviation of a sample (pstdev for standard deviation of population)
output += "Variance: ".ljust(30, ' ') + var_str + "\n" # This calculates the variance of a sample (pvariance for variance of population)
output += "Max: ".ljust(30, ' ') + formatting_style.format(max(values)).rjust(max_number_length, ' ') + "\n"
output += "Min: ".ljust(30, ' ') + formatting_style.format(min(values)).rjust(max_number_length, ' ') + "\n"
output += "\n"

output += "Original raw values:\n"
for i in range(len(values)):
    output += str(values[i]) + "\n"

# Write output to file and end script #
with open(output_file, "w") as f:
    f.write(output)
print("Benchmark script finished")