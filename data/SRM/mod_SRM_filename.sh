#!/bin/bash

# Navigate to the directory containing the files
cd _srm_sp256_vx0.8/

# Loop through files
for file in *; do
    # Check if the file name matches the pattern
    if [[ "$file" =~ ^(.*)_[0-9]{2}$ ]]; then
        # Extract the parts of the file name
        prefix="${BASH_REMATCH[1]}"   # Prefix before the last pair of digits
        last_pair="${file##*_}"       # Last pair of digits
        last_digit="${last_pair: -1}" # Last digit of the last pair
        second_last_digit="${last_pair: -2:1}" # Second last digit of the last pair
        # Rename the file
        mv "$file" "${prefix}_${second_last_digit}_${last_digit}"
        # echo "Renamed $file to ${prefix}_${second_last_digit}_${last_digit}"
    fi
done
