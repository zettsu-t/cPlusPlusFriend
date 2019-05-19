## Based on the example of Google OR-Tools
## https://developers.google.com/optimization/bin/knapsack

import sys
import numpy as np
import pandas as pd
import sqlite3
from ortools.algorithms import pywrapknapsack_solver

def main():
    filename = "saizeriya.db"
    conn = sqlite3.connect(filename)
    df = pd.read_sql_query('SELECT name, price, calorie FROM menu', conn)
    conn.close()

    solver = pywrapknapsack_solver.KnapsackSolver(
        pywrapknapsack_solver.KnapsackSolver.
        KNAPSACK_DYNAMIC_PROGRAMMING_SOLVER,
        'test')

    for total in [1000, 10000]:
        weights = np.asarray(df[['price']]).reshape(1,-1).tolist()
        capacities = [total]
        values = np.asarray(df[['calorie']]).reshape(-1).tolist()
        solver.Init(values, weights, capacities)
        computed_value = solver.Solve()

        packed_items = [x for x in range(0, len(weights[0]))
                        if solver.BestSolutionContains(x)]
        packed_weights = [weights[0][i] for i in packed_items]
        df_seleced = df.loc[packed_items,]
        print(df_seleced)
        print(df_seleced.sum(numeric_only=True))

if __name__ == '__main__':
    main()
