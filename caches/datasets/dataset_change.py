from pandas import read_csv
import numpy as np

for i in range(50):
    full_data = read_csv(f'data{i}.csv', header=None)
    full_data['next_time'] = -1
    ids = full_data[0]
    times = full_data[2]
    all_ids = ids.unique()
    all_dict = {num:np.where(ids == num)[0] for num in all_ids}
    for ind, num in enumerate(ids):
        all_inds = all_dict[num]
        next_ind = all_inds[np.where(all_inds > ind)]
        if next_ind.shape[0] > 0:
            full_data.loc[:, 'next_time'].iloc[ind] = times[next_ind[0]]
    #next_num = next_ind[0] if next_ind.shape[0] > 0 else -1
    #full_data.loc[:, 'next_time'].iloc[ind] = next_num
    #full_data.loc['next_time']
    full_data.to_csv(f'new_data{i}.csv', header=None, index=None)