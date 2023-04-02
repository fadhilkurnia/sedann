#include <iostream>
#include <fstream>
#include <cstdlib>

int main(int argc, char **argv) {
    if (argc != 4) {
        std::cout << "usage: " << argv[0] << "input_bvecs output_smaller_bvecs N" << std::endl; 
        std::cout << "   where N is the prefix size, the first N vectors." << std::endl; 
        exit(-1);
    }

    std::ifstream reader(argv[1], std::ios::binary | std::ios::ate);
    size_t fsize = reader.tellg();
    reader.seekg(0, std::ios::beg);

    uint32_t ndims_u32;
    reader.read((char *)&ndims_u32, sizeof(uint32_t));
    reader.seekg(0, std::ios::beg);
    size_t ndims = (size_t) ndims_u32;
    size_t nvecs = fsize / ((ndims + 4) * sizeof(uint8_t));
    std::cout << ">> Dataset (" << argv[1] <<"): #vector = " << nvecs << ", #dims = " << ndims << std::endl;
    
    size_t nvecs_prefix = atoi(argv[3]);
    std::cout << ">> Getting the first N=" << nvecs_prefix << " vectors ..." << std::endl;

    if (nvecs_prefix == 0) {
        return 0;
    }

    std::ofstream writer(argv[2], std::ios::binary);
    auto read_buf = new uint8_t[nvecs_prefix * (ndims + 4)];
    reader.read((char*) read_buf, nvecs_prefix * (sizeof(uint32_t) + ndims * sizeof(uint8_t)));
    writer.write((char*) read_buf, nvecs_prefix * (4 + ndims * 1));

    reader.close();
    writer.close();

    return 0;
}