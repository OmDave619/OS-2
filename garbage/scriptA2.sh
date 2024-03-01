cpp_file="code2.cpp"
output_file1="timesE2b_bounded.txt"
output_file2="timesE2b_unbounded.txt"
temp_file1="exp2boundedmixed.txt"
temp_file2="exp2unboundedmixed.txt"

g++ -O3 $cpp_file -o try

if [ $? -eq 0 ]; then
    echo "Compilation successful."
else
    echo "Compilation failed."
    exit 1
fi

# Initialize variables to store the sum of outputs for both files
total1=0
total2=0

for i in 1 2 3 4 5; do
    ./try > $temp_file1 2> $temp_file2
    # Read the outputs from the temporary files and add them to their respective totals
    output1=$(cat $temp_file1)
    output2=$(cat $temp_file2)
    total1=$(echo "scale=10; $total1 + $output1" | bc)
    total2=$(echo "scale=10; $total2 + $output2" | bc)
    echo "Run $i completed."
done

# Calculate the averages with decimal values
average1=$(echo "scale=10; $total1 / 5" | bc)
average2=$(echo "scale=10; $total2 / 5" | bc)

# Write the averages to the output files
echo $average1 >> $output_file1
echo $average2 >> $output_file2

echo "All runs completed. Average outputs recorded in $output_file1 and $output_file2."

# Optionally, remove the temporary files
# rm $temp_file1
# rm $temp_file2
