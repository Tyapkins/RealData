import pandas as pd

df = pd.read_csv('./final_info.csv')

all_groups = df.groupby(['length', 'size'])

cache_list = ['lfu', 'lru']

BHR_table = pd.DataFrame(columns=['length', 'size'] + cache_list + ['max_diff', 'min_diff'])
OHR_table = pd.DataFrame(columns=['length', 'size'] + cache_list + ['max_diff', 'min_diff'])

for group in all_groups:
    values = group[1]

    all_BHRs = values['BHR_diff']
    all_OHRs = values['OHR_diff']

    BHR_vals = values['BHR_winner'].value_counts()
    OHR_vals = values['OHR_winner'].value_counts()
    line_to_BHR = [group[0][0], group[0][1]] + [BHR_vals[cache] if cache in BHR_vals else 0 for cache in cache_list] + [all_BHRs.max(), all_BHRs.min()]
    line_to_OHR = [group[0][0], group[0][1]] + [OHR_vals[cache] if cache in OHR_vals else 0 for cache in cache_list] + [all_OHRs.max(), all_OHRs.min()]
    BHR_df = pd.DataFrame([line_to_BHR], columns=BHR_table.columns)
    OHR_df = pd.DataFrame([line_to_OHR], columns=OHR_table.columns)
    BHR_table = pd.concat([BHR_table, BHR_df], ignore_index=True)
    OHR_table = pd.concat([OHR_table, OHR_df], ignore_index=True)
BHR_table.to_csv('two_BHRs.csv', index=False)
OHR_table.to_csv('two_OHRs.csv', index=False)
