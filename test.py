import cuda_outer_product_sum as cu_ops

F = '16798108731015832284940804142231733909889187121439069848933715426072753864723'
rows = 1000
cols = 1000

vec_w = bytearray(rows)
vec_v = bytearray(cols)

adder = cu_ops.cuda_outer_product_sum_solver(rows, cols)
adder.add_to_db(vec_w, vec_v)
adder.reduce()
res = adder.read_results(F.encode(), 5)
