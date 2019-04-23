from libc.stdint cimport uint8_t, int32_t

cdef extern from "cuda_outer_product_sum/include/cuda_outer_product_sum_solver.h":
    cdef cppclass C_cuda_outer_product_sum_solver "cuda_outer_product_sum_solver":
        C_cuda_outer_product_sum_solver(int32_t, int32_t);
        void add_to_db(uint8_t*, uint8_t*)
        void reduce()
        void read_results(uint8_t*, char*, int, int)

cdef class cuda_outer_product_sum_solver:
    cdef int rows
    cdef int cols
    cdef C_cuda_outer_product_sum_solver* g

    def __cinit__(self, rows, cols):
        self.g = new C_cuda_outer_product_sum_solver(rows, cols)
        self.rows = rows
        self.cols = cols

    def add_to_db(self, vec_w, vec_v):
        self.g.add_to_db(vec_w, vec_v)

    def reduce(self):
        self.g.reduce()

    def read_results(self, modulus, threads):
        result = bytearray(self.rows*self.cols)
        self.g.read_results(result, modulus, 10, threads)
