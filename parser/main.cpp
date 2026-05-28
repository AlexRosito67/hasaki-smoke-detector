#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <random>

float minValue[] = {-22.01f, 10.74f, 0.0f, 400.0f, 10668.0f, 15317.0f, 930.852f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
float maxValue[] = {59.93f, 75.2f, 60000.0f, 60000.0f, 13803.0f, 21410.0f, 939.861f, 14333.69f, 45432.26f, 61482.03f, 51914.68f, 30026.438f};

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <input_file>\n";
        return 1;
    }

    std::ifstream infile(argv[1]);
    if (!infile.is_open())
    {
        std::cerr << "Error opening file: " << argv[1] << '\n';
        return 1;
    }

    std::vector<std::vector<float>> class0_rows;
    std::vector<std::vector<float>> class1_rows;

    std::string line;
    bool first_title_line = true;

    while (std::getline(infile, line))
    {
        if (first_title_line)
        {
            first_title_line = false;
            continue;
        }

        std::istringstream iss(line);
        std::string value;
        std::vector<float> values;
        int index = 0;

        while (std::getline(iss, value, ','))
        {
            if (index < 12)
                values.push_back((std::stof(value) - minValue[index]) / (maxValue[index] - minValue[index]));
            else
                values.push_back(std::stof(value));
            index++;
        }

        if (values.empty()) continue;

        int label = (int)values.back();
        if (label == 0)
            class0_rows.push_back(values);
        else
            class1_rows.push_back(values);
    }
    infile.close();

    std::cout << "Class 0: " << class0_rows.size() << " rows\n";
    std::cout << "Class 1: " << class1_rows.size() << " rows\n";

    // Undersample clase 1 hasta igualar clase 0
    size_t target_size = class0_rows.size();
    if (class1_rows.size() > target_size)
    {
        std::mt19937 rng(42);
        std::shuffle(class1_rows.begin(), class1_rows.end(), rng);
        class1_rows.resize(target_size);
    }

    std::cout << "After undersample - Class 0: " << class0_rows.size()
              << " | Class 1: " << class1_rows.size() << "\n";

    // Combinar y mezclar
    std::vector<std::vector<float>> all_rows;
    all_rows.insert(all_rows.end(), class0_rows.begin(), class0_rows.end());
    all_rows.insert(all_rows.end(), class1_rows.begin(), class1_rows.end());

    std::mt19937 rng2(123);
    std::shuffle(all_rows.begin(), all_rows.end(), rng2);

    std::ofstream outfile("output.csv");
    if (!outfile.is_open())
    {
        std::cerr << "Error creating output file\n";
        return 1;
    }

    for (size_t i = 0; i < all_rows.size(); i++)
    {
        for (size_t j = 0; j < all_rows[i].size(); j++)
        {
            outfile << all_rows[i][j];
            if (j < all_rows[i].size() - 1)
                outfile << ",";
        }
        outfile << "\n";
    }

    outfile.close();
    std::cout << "Total rows written: " << all_rows.size() << "\n";
    std::cout << "Processing complete. Output written to output.csv\n";
    return 0;
}
