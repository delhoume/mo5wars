#include <fstream>
#include <iostream>
#include <random>
#include <string>

int main(int argc, char *argv[]) {
  // 1. Check if both input and output file arguments were provided
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0]
              << " <input_file_path> <output_file_path67> [<output_file_path64>] \n";
    return 1;
  }

  std::string input_filename = argv[1];
  std::string output_filename =
      argv[2]; // Captures the named output file from command line

  // 2. Open input file
  std::ifstream infile(input_filename, std::ios::binary);
  if (!infile.is_open()) {
    std::cerr << "Error: Could not open input file: " << input_filename << "\n";
    return 1;
  }

  // 3. Open output file with custom runtime name
  std::ofstream outfile(output_filename, std::ios::binary);
  if (!outfile.is_open()) {
    std::cout << "Error: Could not create output file: " << output_filename
              << "\n";
    return 1;
  }
 
  std::string line;
  std::string line64;
  std::string line67;
  int line_number = 0;
  // 4. Read file line by line
  while (std::getline(infile, line)) {
    std::cout << "Processing line " << line_number + 1 << ": " << line.length() << line <<  "\n";
    line_number++;
    std::string processed;
    if (line.length() < 67) {
      processed = line.append(67 - line.length(), ' ');
    } else {
  processed = line;
    }
    line67 += processed;
    line64 += processed.substr(1 , 64);
  }
 infile.close();
  outfile << line67;
  outfile.close();
  if (argc > 3) {
    std::ofstream outfile64(argv[3], std::ios::binary);
    if (!outfile64.is_open()) {
      std::cout << "Error: Could not create output file: " << argv[3] << "\n";
      return 1;
    }
    outfile64 << line64;
    outfile64.close();
  } 
  
  std::cout << "Success! Stream flattened and saved to: " << output_filename
            << "\n";
  return 0;
}
