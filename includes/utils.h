float* read_bvecs(char* filename);
float* read_fvecs(char* filename);
uint32_t* read_ivecs(char* filename);

void save_ivecs(uint32_t* data, uint32_t dim, char* filename);
