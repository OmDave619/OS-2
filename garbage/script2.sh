cpp_file="code2.cpp"
output_file="timesE1b.txt"
temp_file="output2.txt"

g++ -O3 $cpp_file -o try2

if [ $? -eq 0 ]; then
    echo "Compilation successful."
else
    echo "Compilation failed."
    exit 1
fi

# Initialize a variable to store the sum of outputs
total=0

for i in 1 2 3 4 5; do
    ./try2 > $temp_file
    # Read the output from the temporary file and add it to the total
    output=$(cat $temp_file)
    total=$(echo "scale=10; $total + $output" | bc)
    echo "Run $i completed."
done

# Calculate the average with decimal values
average=$(echo "scale=10; $total / 5" | bc)

# Write the average to the output file
echo  $average >> $output_file

echo "All runs completed. Average output recorded in $output_file."