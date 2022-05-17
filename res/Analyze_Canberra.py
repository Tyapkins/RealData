import pandas as pd
import os
from tqdm import tqdm
import numpy as np

canberra_res = pd.read_csv("canberra_res.csv")
all_metrics_res = pd.read_csv("three_metrics.csv")
#print(canberra_res)
all_metrics_res_groups = all_metrics_res.groupby('metric')

canberra_groups = canberra_res.groupby(['length', 'cache size', 'datatype', 'neigh. num'])

can_group = None
for group in all_metrics_res_groups:
        if (group[0] == 'canberra'):
                can_group = group[1].drop(['metric'], axis=1)


metrics_groups = can_group.groupby(['length', 'size', 'datatype', 'NeighNum'])
new_dataframe = pd.DataFrame(columns=['length', 'size', 'datatype', 'NeighNum', 'test_score', 'dim', 'NN_test_score'])

for group in metrics_groups:
        canberra_group = canberra_groups.get_group(group[0])
        #print(canberra_group)
        #print(group[1]['res_test (mean)'].values[0])
        for dim in canberra_group['red. length']:
                current_res = pd.DataFrame({'length': group[0][0], 'size':  group[0][1], 'datatype': group[0][2], 'NeighNum': group[0][3], 
                'test_score': group[1]['res_test (mean)'].values[0], 'dim': dim, 
                'NN_test_score': canberra_group.where(canberra_group['red. length'] == dim)['test score'].dropna().values[0]}, index=[1])
                new_dataframe = pd.concat([new_dataframe, current_res], ignore_index=True)
            
            
#print(new_dataframe)
new_dataframe.to_csv('compare_res.csv', index=False)
