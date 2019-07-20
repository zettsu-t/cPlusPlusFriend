import sys
import numpy as np
df = 6
vec_size = int(sys.argv[1]) if len(sys.argv) > 1 else 250000
vec = np.random.standard_t(df=df, size=vec_size)
out_filename = 'axes_compile_time_py.inc'

with open(out_filename, 'w') as file:
    file.write('std::vector<double> data ')
    file.write(np.array2string(a=vec, max_line_width=1000000000, precision=6,
                               separator=',', suppress_small=True,
                               threshold=np.nan, floatmode='fixed').
               translate(str.maketrans('[]', '{}')))
    file.write(';\n\n')
