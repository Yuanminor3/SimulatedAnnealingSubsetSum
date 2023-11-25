import matplotlib.pyplot as plt
import numpy as np


# Define the path to your log file
log_file_path = 'anneal.log'

# Initialize lists to store the extracted data for plotting
x_values = []
y_values = []

# Open the file and read the lines
with open(log_file_path, 'r') as file:
    # Skip the header line
    next(file)
    # Loop over the file line by line
    for line in file:
        # Break the loop if an empty line is encountered
        if line.strip() == "":
            break
        # Split the line into components using comma as the delimiter
        parts = line.split(',')
        # Append the first column to x_values and the third column to y_values
        x_values.append(int(parts[0]))
        y_values.append(float(parts[2]))




# Plotting
plt.figure(figsize=(10, 8))  # Same size as the example image
plt.plot(x_values, y_values, 'r')  # Red line plot
plt.title('Cost Function',fontsize = 20)
plt.xlabel('Step', fontsize = 20)
plt.ylabel('Cost Value', fontsize = 20)
plt.grid(True)

plt.xticks(range(0, max(x_values) + 1, 10))
max_y_value = max(y_values)
y_ticks = np.arange(0, max_y_value, 50000)
plt.yticks(y_ticks)


plt.tight_layout()
plt.show()