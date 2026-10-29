#include <stdio.h>
#include <stdlib.h>

#define MAX_LINE_LENGTH 256
#define INITIAL_CAPACITY 10

int main(int argc, char *argv[]) {
    // Default file names
    const char *input_filename = "numbers.txt";
    const char *output_filename = "delays.h";

    // Handle command-line arguments
    if (argc >= 2) {
        input_filename = argv[1];
    }
    if (argc >= 3) {
        output_filename = argv[2];
    }
    if (argc > 3) {
        fprintf(stderr, "Usage: %s [input_file] [output_file]\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("Reading from: %s\n", input_filename);
    printf("Writing to:   %s\n", output_filename);

    // Open the source data file
    FILE *source_file = fopen(input_filename, "r");
    if (source_file == NULL) {
        perror("Error opening input file");
        return EXIT_FAILURE;
    }

    // Allocate dynamic array memory to store values using unsigned char
    int capacity = INITIAL_CAPACITY;
    int count = 0;
    unsigned char *values = malloc(capacity * sizeof(unsigned char));
    if (values == NULL) {
        perror("Memory allocation failed");
        fclose(source_file);
        return EXIT_FAILURE;
    }

    char line[MAX_LINE_LENGTH];
    int temp_val;

    // Read the file line-by-line
    while (fgets(line, sizeof(line), source_file) != NULL) {
        if (sscanf(line, "%d", &temp_val) == 1) {
            
            // Value safety check for 8-bit unsigned integer limits (0-255)
            if (temp_val < 0 || temp_val > 255) {
                fprintf(stderr, "Warning: Value %d out of 8-bit range on line %d. Capping.\n", temp_val, count + 1);
                if (temp_val < 0) temp_val = 0;
                if (temp_val > 255) temp_val = 255;
            }

            // Resize array dynamically if full
            if (count >= capacity) {
                capacity *= 2;
                unsigned char *temp_ptr = realloc(values, capacity * sizeof(unsigned char));
                if (temp_ptr == NULL) {
                    perror("Memory reallocation failed");
                    free(values);
                    fclose(source_file);
                    return EXIT_FAILURE;
                }
                values = temp_ptr;
            }
            
            values[count] = (unsigned char)temp_val;
            count++;
        }
    }
    fclose(source_file);

    // Open the target include file (.h) for writing
    FILE *header_file = fopen(output_filename, "w");
    if (header_file == NULL) {
        perror("Error creating output header file");
        free(values);
        return EXIT_FAILURE;
    }

    // Write standard header guards and array definition (No stdint.h inclusion)
    fprintf(header_file, "#ifndef DELAYS_H\n");
    fprintf(header_file, "#define DELAYS_H\n\n");
    fprintf(header_file, "#define DELAYS_ARRAY_SIZE %d\n\n", count);
    fprintf(header_file, "static const unsigned char delays[DELAYS_ARRAY_SIZE] = {\n");

    for (int i = 0; i < count; i++) {
        fprintf(header_file, "    %u%s\n", values[i], (i < count - 1) ? "," : "");
    }

    fprintf(header_file, "};\n\n");
    fprintf(header_file, "#endif // DELAYS_H\n");
    fclose(header_file);

    printf("Successfully generated '%s' with %d values.\n", output_filename, count);

    free(values);
    return EXIT_SUCCESS;
}
