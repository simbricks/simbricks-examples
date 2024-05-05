import random

def rand_matrix(M, N):
    return [[random.randint(0,255) for _ in range(M)] for _ in range(N)]

def zero_matrix(M, N):
    return [[0] * M] * N

def matmult(A, B):
    C = zero_matrix(len(A), len(B[0]))
    for i in range(len(A)):
       for j in range(len(B[0])):
           for k in range(len(B)):
               C[i][j] = (C[i][j] + A[i][k] * B[k][j]) % 256
    return C


N = 200

A = rand_matrix(N, N)
B = rand_matrix(N, N)

matmult(A, B)
