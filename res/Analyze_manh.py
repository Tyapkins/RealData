import pandas as pd
import os
from tqdm import tqdm
import numpy as np

manh_res = pd.read_csv("manh.csv")
all_metrics_res = pd.read_csv("three_metrics.csv")
all_metrics_res_groups = all_metrics_res.groupby('metric')

manh_groups = manh_res.groupby(['length', 'cache size', 'datatype', 'neigh. num'])

man_group = None
for group in all_metrics_res_groups:
        if (group[0] == 'euclidean'):
                man_group = group[1].drop(['metric'], axis=1)


metrics_groups = man_group.groupby(['length', 'size', 'datatype', 'NeighNum'])
new_dataframe = pd.DataFrame(columns=['length', 'size', 'datatype', 'NeighNum', 'test_score', 'dim', 'NN_test_score'])

for group in manh_groups:
        cur_group = metrics_groups.get_group(group[0])
        for dim in group[1]['red. length']:
                current_res = pd.DataFrame({'length': group[0][0], 'size':  group[0][1], 'datatype': group[0][2], 'NeighNum': group[0][3], 
                'test_score': cur_group['res_test (mean)'].values[0], 'dim': dim, 
                'NN_test_score': group[1].where(group[1]['red. length'] == dim)['test score'].dropna().values[0]}, index=[1])
                new_dataframe = pd.concat([new_dataframe, current_res], ignore_index=True)

new_dataframe.to_csv('compare_eucl.csv', index=False)
