import sys
import pathlib

def read_ellpack(path):
    with open(path) as f:
        rows, cols, k = map(int, f.readline().strip().split(","))
        vals = f.readline().strip().split(",")
        inds = f.readline().strip().split(",")
    # Build dense matrix by columns
    M = [[0.0 for _ in range(cols)] for _ in range(rows)]
    pos = 0
    for c in range(cols):
        for _ in range(k):
            v = vals[pos]; i = inds[pos]; pos += 1
            if v != "*" and i != "*":
                r = int(i)
                M[r][c] = float(v)
    return M

def read_result_ellpack(path):
    with open(path) as f:
        rows, cols, k = map(int, f.readline().strip().split(","))
        vals = f.readline().strip().split(",")
        inds = f.readline().strip().split(",")
    M = [[0.0 for _ in range(cols)] for _ in range(rows)]
    pos = 0
    for c in range(cols):
        for _ in range(k):
            v = vals[pos]; i = inds[pos]; pos += 1
            if v != "*" and i != "*":
                r = int(i)
                M[r][c] = float(v)
    return M

def matmul_dense(A, B):
    rows_a = len(A)
    cols_a = len(A[0]) if rows_a else 0
    rows_b = len(B)
    cols_b = len(B[0]) if rows_b else 0
    if cols_a != rows_b:
        raise ValueError("shape mismatch: A is %dx%d, B is %dx%d" % (rows_a, cols_a, rows_b, cols_b))
    R = [[0.0 for _ in range(cols_b)] for _ in range(rows_a)]
    for i in range(rows_a):
        for k in range(cols_a):
            aik = A[i][k]
            if aik == 0.0:
                continue
            for j in range(cols_b):
                R[i][j] += aik * B[k][j]
    return R

def allclose(R, Ref, atol=1e-6, rtol=1e-6):
    rows = len(R)
    cols = len(R[0]) if rows else 0
    for i in range(rows):
        for j in range(cols):
            diff = abs(R[i][j] - Ref[i][j])
            tol = atol + rtol * abs(Ref[i][j])
            if diff > tol:
                return False
    return True

def main():
    if len(sys.argv) != 4:
        print("Usage: check_result.py <A.ellpack> <B.ellpack> <result.ellpack>")
        sys.exit(1)
    A = read_ellpack(sys.argv[1])
    B = read_ellpack(sys.argv[2])
    R = read_result_ellpack(sys.argv[3])
    ref = matmul_dense(A, B)
    ok = allclose(R, ref, atol=1e-6, rtol=1e-6)
    print("Shape A:", (len(A), len(A[0]) if A else 0), "B:", (len(B), len(B[0]) if B else 0), "R:", (len(R), len(R[0]) if R else 0))
    if ok:
        print("OK: Result matches NumPy reference within tolerance.")
        sys.exit(0)
    else:
        # Show a small excerpt and max diff
        max_diff = 0.0
        for i in range(min(5, len(R))):
            for j in range(min(5, len(R[0]) if R else 0)):
                max_diff = max(max_diff, abs(R[i][j] - ref[i][j]))
        print("Mismatch. Max abs diff:", max_diff)
        print("R (excerpt):")
        for i in range(min(5, len(R))):
            print(R[i][:min(5, len(R[0]) if R else 0)])
        print("Ref (excerpt):")
        for i in range(min(5, len(ref))):
            print(ref[i][:min(5, len(ref[0]) if ref else 0)])
        sys.exit(2)

if __name__ == "__main__":
    main()